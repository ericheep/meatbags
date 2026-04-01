//
//  OrbbecPulsar.cpp
//  meatbags

#include "OrbbecPulsar.hpp"

OrbbecPulsar::OrbbecPulsar() : Sensor() {
	angularResolution = int(360.0 / 0.1);
	port = 2228;
	rotationFrequency = 20;
	showSensorInformation = false;

	currentRotationSpeed = 0;
	temperature = 0.0;
	timestamp = 0;
	lidarFaultCode = 0;
	lidarFaultDescription = "";
	consecutiveErrorFrames = 0;
	consecutiveWarningFrames = 0;

	setupParameters();

	statusCommands.clear();
	statusCommands.push_back([this]() { sendGetMotorSpeedCommand(); });
	statusCommands.push_back([this]() { sendGetTransmissionProtocolCommand(); });
	statusCommands.push_back([this]() { sendGetWorkingModeCommand(); });
	statusCommands.push_back([this]() { sendGetSerialNumberCommand(); });
	statusCommands.push_back([this]() { sendGetDeviceModelCommand(); });
	statusCommands.push_back([this]() { sendGetFirmwareVersionCommand(); });
	statusCommands.push_back([this]() { sendGetLidarWarningCommand(); });
	statusCommands.push_back([this]() { sendGetSpecialWorkingModeCommand(); });
	currentStatusCommandIndex = 0;

	checkCommands.clear();
	checkCommands.push_back([this]() { checkMotorSpeed(); });
	checkCommands.push_back([this]() { checkTransmissionProtocol(); });
	checkCommands.push_back([this]() { checkWorkingMode(); });
	checkCommands.push_back([this]() { checkSpecialWorkingMode(); });
	currentCheckCommandIndex = 0;

	commandBuffer.reserve(64);
}

OrbbecPulsar::~OrbbecPulsar() {
	statusCommands.clear();
	checkCommands.clear();

	sendDisableDataStreamCommand();
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

void OrbbecPulsar::threadedFunction() {
	initializeVectors();

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	bool tcpConnected = tcpSetup();
	isConnected = tcpConnected;

	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	if (tcpConnected) {
		// always disable data stream first to clear any stale state from
		// a previous session that didn't shut down cleanly (e.g. flashing red)
		ofLogNotice("Orbbec") << "Sensor " << index << " clearing stale state...";
		sendDisableDataStreamCommand();
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		sendSetRangingMode();
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		sendConnectCommand();
	}

	// accumulation buffer for TCP stream reassembly
	vector<uint8_t> accumBuffer;
	accumBuffer.reserve(4096);

	auto lastDataTime = std::chrono::steady_clock::now();
	const auto timeoutDuration = std::chrono::seconds(30);

	auto lastStatusTime = std::chrono::steady_clock::now();
	const auto statusInterval = std::chrono::milliseconds(199);

	auto lastCheckSettingTime = std::chrono::steady_clock::now();
	const auto checkSettingInterval = std::chrono::milliseconds(200);

	auto lastModeResetTime = std::chrono::steady_clock::now() - std::chrono::seconds(30);
	const auto modeResetCooldown = std::chrono::seconds(5);

	auto lastReconnectionTime = std::chrono::steady_clock::now();
	const auto reconnectionTimeout = std::chrono::milliseconds(5000);

	while (isThreadRunning()) {
		// read incoming bytes under mutex and append to accumulation buffer
		int bytesRead = 0;
		{
			std::lock_guard<std::mutex> lock(tcpMutex);
			isConnected = tcpClient.isConnected();
			bytesRead = tcpClient.receiveRawBytes((char*)receiveBuffer, sizeof(receiveBuffer));
		}

		if (bytesRead > 0) {
			accumBuffer.insert(accumBuffer.end(), receiveBuffer, receiveBuffer + bytesRead);
			lastDataTime = std::chrono::steady_clock::now();
		}

		// process all complete frames from accumulation buffer
		while (accumBuffer.size() >= 10) {
			if (accumBuffer[0] == 0x01 && accumBuffer[1] == 0xFE) {
				// control response: header(2) + version(1) + dataLen(2) + controlCode(2) + responseCode(2) + data + crc(1)
				uint16_t dataLength = (accumBuffer[3] << 8) | accumBuffer[4];
				size_t frameSize = 10 + dataLength;

				if (accumBuffer.size() < frameSize) break; // wait for more bytes

				parseControlResponse(accumBuffer.data(), (int)frameSize);
				accumBuffer.erase(accumBuffer.begin(), accumBuffer.begin() + frameSize);
			}
			else if (accumBuffer.size() >= 8 &&
				accumBuffer[0] == 0x4D && accumBuffer[1] == 0x53 && accumBuffer[2] == 0x02 &&
				accumBuffer[3] == 0xF4 && accumBuffer[4] == 0xEB && accumBuffer[5] == 0x90) {
				// point cloud: frameLength is in bytes 6-7
				uint16_t frameLength = (accumBuffer[6] << 8) | accumBuffer[7];

				if (accumBuffer.size() < frameLength) break; // wait for more bytes

				parsePointCloudData(accumBuffer.data(), (int)frameLength);
				accumBuffer.erase(accumBuffer.begin(), accumBuffer.begin() + frameLength);
			}
			else {
				// unrecognized byte at front, discard and resync
				accumBuffer.erase(accumBuffer.begin());
			}
		}

		auto now = std::chrono::steady_clock::now();
		if (now - lastDataTime > timeoutDuration) {
			ofLogNotice("Orbbec") << "Resending connect command";
			sendConnectCommand();
			lastDataTime = std::chrono::steady_clock::now();
		}

		if (now - lastStatusTime > statusInterval) {
			sendNextStatusCommand();
			lastStatusTime = std::chrono::steady_clock::now();
		}

		if (now - lastCheckSettingTime > checkSettingInterval) {
			checkNextSetting();
			lastCheckSettingTime = std::chrono::steady_clock::now();
		}

		// react to persistent error state — standby→ranging mode reset, gated by cooldown
		int errFrames;
		{ std::lock_guard<std::mutex> lock(sensorDataMutex); errFrames = consecutiveErrorFrames; }

		if (errFrames >= ERROR_RECONNECT_THRESHOLD &&
			now - lastModeResetTime > modeResetCooldown) {
			string fault;
			{ std::lock_guard<std::mutex> lock(sensorDataMutex); fault = lidarFaultDescription; }
			if (fault.empty()) fault = "unknown fault";

			ofLogWarning("Orbbec") << "Sensor " << index
				<< " persistent error (" << fault << ") — mode reset (standby→ranging)";

			// reset immediately so we don't re-trigger before the cooldown expires
			{ std::lock_guard<std::mutex> lock(sensorDataMutex); consecutiveErrorFrames = 0; }

			sendSetStandbyMode();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			sendSetRangingMode();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			sendEnableDataStreamCommand();
			accumBuffer.clear();
			lastModeResetTime = std::chrono::steady_clock::now();
		}

		bool connected;
		{
			std::lock_guard<std::mutex> lock(tcpMutex);
			connected = tcpClient.isConnected();
		}

		if (!connected) {
			if (now - lastReconnectionTime > reconnectionTimeout) {
				ofLogNotice("Orbbec") << "Connection lost, attempting reconnection";

				if (tcpSetup()) {
					ofLogNotice("Orbbec") << "Reconnection successful";
					accumBuffer.clear(); // discard stale partial data on reconnect
					sendConnectCommand();
				}
				else {
					ofLogNotice("Orbbec") << "Reconnection unsuccessful";
				}

				lastReconnectionTime = std::chrono::steady_clock::now();
			}
		}
		else {
			lastReconnectionTime = std::chrono::steady_clock::now();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// graceful shutdown — send disable while socket is still open
	ofLogNotice("Orbbec") << "Sensor " << index << " shutting down, disabling data stream";
	sendDisableDataStreamCommand();
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

bool OrbbecPulsar::isPointCloudData(const uint8_t* data, int bytesRead) {
	return (bytesRead >= 44 &&
		data[0] == 0x4D && data[1] == 0x53 && data[2] == 0x02 &&
		data[3] == 0xF4 && data[4] == 0xEB && data[5] == 0x90);
}

bool OrbbecPulsar::isControlResponse(const uint8_t* data, int bytesRead) {
	return (bytesRead >= 10 &&
		data[0] == 0x01 && data[1] == 0xFE &&
		bytesRead < 100);
}

void OrbbecPulsar::update() {
	updateDistances();
	updateSensorInfo();
}

void OrbbecPulsar::updateSensorInfo() {
	if (!showSensorInformation) return;

	sensorInfoLines.clear();
	sensorInfoLines.reserve(15);

	{
		std::lock_guard<std::mutex> lock(sensorDataMutex);
		sensorInfoLines.emplace_back("Model: " + model);
		sensorInfoLines.emplace_back("Firmware: " + firmwareVersion);
		sensorInfoLines.emplace_back("Serial: " + serialNumber);
		sensorInfoLines.emplace_back("LiDAR state: " + lidarState);
		sensorInfoLines.emplace_back("Transmission Protocol: " + transmissionProtocol);
		sensorInfoLines.emplace_back("Set Motor Speed: " + to_string(motorSpeed) + " RPM");
		sensorInfoLines.emplace_back("Real Time Motor Speed: " + to_string(currentRotationSpeed) + " RPM");
		sensorInfoLines.emplace_back("Temperature: " + to_string(temperature) + "°C");
		sensorInfoLines.emplace_back("IP Address: " + ipAddress.get());
		sensorInfoLines.emplace_back("Port: " + to_string(port));
		sensorInfoLines.emplace_back("Connection: " + connectionStatus);
		sensorInfoLines.emplace_back("Timestamp: " + to_string(timestamp));
		sensorInfoLines.emplace_back("Working Mode: " + workingMode);
		sensorInfoLines.emplace_back("Warning: " + lidarWarning);
		sensorInfoLines.emplace_back("Special Working Mode: " + specialWorkingMode);
	}

	logStatus = lidarState;
	logConnectionStatus = connectionStatus;
	logMode = workingMode;
}

void OrbbecPulsar::checkNextSetting() {
	if (checkCommands.empty()) return;
	checkCommands[currentCheckCommandIndex]();
	currentCheckCommandIndex = (currentCheckCommandIndex + 1) % checkCommands.size();
}

void OrbbecPulsar::checkMotorSpeed() {
	// guiMotorSpeed stores Hz directly: 15, 20, 25, 30, 40
	uint32_t speedValue;
	switch (guiMotorSpeed.get()) {
	case 15: speedValue = 900;  break;
	case 20: speedValue = 1200; break;
	case 25: speedValue = 1500; break;
	case 30: speedValue = 1800; break;
	case 40: speedValue = 2400; break;
	default: speedValue = 1200; break;  // default 20Hz
	}

	if (speedValue != (uint32_t)motorSpeed) {
		sendSetMotorSpeedCommand(speedValue);
	}
}

void OrbbecPulsar::checkWorkingMode() {
	if (isSleeping && workingMode != "standby") {
		sendSetStandbyMode();
	}
	if (!isSleeping && workingMode != "ranging") {
		sendSetRangingMode();
	}
}

void OrbbecPulsar::checkTransmissionProtocol() {
	if (transmissionProtocol != "TCP") {
		sendSetTCPModeCommand();
	}
}

void OrbbecPulsar::checkSpecialWorkingMode() {
	if (!guiSpecialWorkingMode && specialWorkingMode != "normal") {
		sendSetNormalModeCommand();
	}
	if (guiSpecialWorkingMode && specialWorkingMode != "fog") {
		sendSetFogModeCommand();
	}
}

void OrbbecPulsar::sendGetMotorSpeedCommand() {
	sendGetCommand(GET_MOTOR_SPEED);
}

void OrbbecPulsar::sendGetTransmissionProtocolCommand() {
	sendGetCommand(GET_TRANS_PROTOCOL);
}

void OrbbecPulsar::sendGetWorkingModeCommand() {
	sendGetCommand(GET_WORKING_MODE);
}

void OrbbecPulsar::sendGetSerialNumberCommand() {
	sendGetCommand(GET_DEVICE_SERIAL);
}

void OrbbecPulsar::sendGetDeviceModelCommand() {
	sendGetCommand(GET_DEVICE_MODEL);
}

void OrbbecPulsar::sendGetFirmwareVersionCommand() {
	sendGetCommand(GET_FIRMWARE_VERSION);
}

void OrbbecPulsar::sendGetLidarWarningCommand() {
	sendGetCommand(GET_LIDAR_WARNING);
}

void OrbbecPulsar::sendGetSpecialWorkingModeCommand() {
	sendGetCommand(GET_SPECIFIC_MODE);
}

void OrbbecPulsar::sendNextStatusCommand() {
	if (statusCommands.empty()) return;
	statusCommands[currentStatusCommandIndex]();
	currentStatusCommandIndex = (currentStatusCommandIndex + 1) % statusCommands.size();
}

void OrbbecPulsar::sendGetCommand(uint16_t getCommand) {
	commandBuffer.resize(10);

	commandBuffer[0] = 0x01;
	commandBuffer[1] = 0xFE;
	commandBuffer[2] = 0x01;
	commandBuffer[3] = 0x00;
	commandBuffer[4] = 0x00;
	commandBuffer[5] = (uint8_t)((getCommand >> 8) & 0xFF);
	commandBuffer[6] = (uint8_t)(getCommand & 0xFF);
	commandBuffer[7] = 0x00;
	commandBuffer[8] = 0x00;

	uint8_t crc = calculateCRC8Fast(commandBuffer.data(), 9);
	commandBuffer[9] = crc;

	sendControlCommand(commandBuffer);
}

void OrbbecPulsar::sendConnectCommand() {
	sendControlMessage(REG_CONNECT_DEVICE, { 0x12, 0x34, 0x56, 0x78 });
}

void OrbbecPulsar::sendEnableDataStreamCommand() {
	sendControlMessage(REG_SET_DATA_STREAM, { 0x00, 0x00, 0x00, 0x01 });
}

void OrbbecPulsar::sendDisableDataStreamCommand() {
	sendControlMessage(REG_SET_DATA_STREAM, { 0x00, 0x00, 0x00, 0x00 });
}

void OrbbecPulsar::sendSetStandbyMode() {
	sendControlMessage(REG_SET_WORKING_MODE, { 0x00, 0x00, 0x00, 0x01 });
}

void OrbbecPulsar::sendSetRangingMode() {
	sendControlMessage(REG_SET_WORKING_MODE, { 0x00, 0x00, 0x00, 0x00 });
}

void OrbbecPulsar::sendSetNormalModeCommand() {
	sendControlMessage(REG_SET_SPECIAL_WORKING_MODE, { 0x00, 0x00, 0x00, 0x00 });
}

void OrbbecPulsar::sendSetFogModeCommand() {
	sendControlMessage(REG_SET_SPECIAL_WORKING_MODE, { 0x00, 0x00, 0x00, 0x01 });
}

void OrbbecPulsar::sendSetMotorSpeedCommand(int speed) {
	switch (speed) {
	case 900: break;
	case 1200: break;
	case 1500: break;
	case 1800: break;
	case 2400: break;
	default: return; break;
	}

	sendControlMessage(REG_SET_MOTOR_SPEED, {
		(uint8_t)((speed >> 24) & 0xFF),
		(uint8_t)((speed >> 16) & 0xFF),
		(uint8_t)((speed >> 8) & 0xFF),
		(uint8_t)(speed & 0xFF)
		});
}

void OrbbecPulsar::sendSetTCPModeCommand() {
	commandBuffer.resize(14);

	commandBuffer[0] = 0x01;
	commandBuffer[1] = 0xFE;
	commandBuffer[2] = 0x01;
	commandBuffer[3] = 0x00;
	commandBuffer[4] = 0x04;
	commandBuffer[5] = 0x01;
	commandBuffer[6] = 0x07;
	commandBuffer[7] = 0x00;
	commandBuffer[8] = 0x00;
	commandBuffer[9] = 0x00;
	commandBuffer[10] = 0x00;
	commandBuffer[11] = 0x00;
	commandBuffer[12] = 0x01;

	uint8_t crc = calculateCRC8Fast(commandBuffer.data(), 13);
	commandBuffer[13] = crc;

	sendControlCommand(commandBuffer);
}

void OrbbecPulsar::sendControlMessage(uint16_t registerAddr, std::initializer_list<uint8_t> data) {
	size_t dataLength = data.size();
	size_t totalSize = dataLength + 9 + 1;
	commandBuffer.resize(totalSize);

	commandBuffer[0] = 0x01;
	commandBuffer[1] = 0xFE;
	commandBuffer[2] = 0x01;
	commandBuffer[3] = (uint8_t)((dataLength >> 8) & 0xFF);
	commandBuffer[4] = (uint8_t)(dataLength & 0xFF);
	commandBuffer[5] = (uint8_t)((registerAddr >> 8) & 0xFF);
	commandBuffer[6] = (uint8_t)(registerAddr & 0xFF);
	commandBuffer[7] = 0x00;
	commandBuffer[8] = 0x00;

	if (dataLength > 0) {
		std::copy(data.begin(), data.end(), commandBuffer.begin() + 9);
	}

	uint8_t crc = calculateCRC8Fast(commandBuffer.data(), totalSize - 1);
	commandBuffer[totalSize - 1] = crc;

	sendControlCommand(commandBuffer);
}

void OrbbecPulsar::sendControlCommand(const vector<uint8_t>& command) {
	std::lock_guard<std::mutex> lock(tcpMutex);
	if (tcpClient.isConnected()) {
		tcpClient.sendRawBytes((char*)command.data(), command.size());
	}
}

void OrbbecPulsar::parseControlResponse(const uint8_t* data, int bytesRead) {
	int16_t controlCode = bytesToUint16(data[5], data[6]);
	int16_t responseCode = bytesToUint16(data[7], data[8]);

	bool success = (responseCode == 0x0100);

	if (success) {
		switch (controlCode) {
		case REG_CONNECT_DEVICE: {
			sendSetTCPModeCommand();
			break;
		}
		case REG_SET_TCP_MODE: {
			sendEnableDataStreamCommand();
			break;
		}
		case REG_SET_DATA_STREAM: {
			break;
		}
		case REG_SET_MOTOR_SPEED: {
			break;
		}
		case REG_SET_WORKING_MODE: {
			break;
		}
		case GET_MOTOR_SPEED: {
			parseMotorSpeed(data, bytesRead);
			break;
		}
		case GET_TRANS_PROTOCOL: {
			parseTransmissionProtocol(data, bytesRead);
			break;
		}
		case GET_WORKING_MODE: {
			parseWorkingMode(data, bytesRead);
			break;
		}
		case GET_DEVICE_SERIAL: {
			parseSerialNumber(data, bytesRead);
			break;
		}
		case GET_DEVICE_MODEL: {
			parseDeviceModel(data, bytesRead);
			break;
		}
		case GET_FIRMWARE_VERSION: {
			parseFirmwareVersion(data, bytesRead);
			break;
		}
		case GET_LIDAR_WARNING: {
			parseLidarWarning(data, bytesRead);
			break;
		}
		case GET_SPECIFIC_MODE: {
			parseSpecialWorkingMode(data, bytesRead);
			break;
		}
		default: break;
		}
	}
}

void OrbbecPulsar::parseString(const uint8_t* data, int bytesRead, string& value) {
	uint16_t dataLength = bytesToUint16(data[3], data[4]);

	int actualLength = dataLength;
	while (actualLength > 0 && (data[8 + actualLength] == 0 || std::isspace(data[8 + actualLength]))) {
		actualLength--;
	}

	string raw(reinterpret_cast<const char*>(data + 9), actualLength);

	// strip non-printable characters (handles embedded nulls, control bytes, high bytes)
	string cleaned;
	cleaned.reserve(raw.size());
	for (unsigned char c : raw) {
		if (c >= 32 && c < 127) cleaned += c;
	}

	std::lock_guard<std::mutex> lock(sensorDataMutex);
	value = cleaned;
}

void OrbbecPulsar::parseTransmissionProtocol(const uint8_t* data, int bytesRead) {
	uint16_t dataLength = bytesToUint16(data[3], data[4]);

	if (dataLength == 0 || bytesRead < (9 + dataLength)) {
		ofLogWarning("OrbbecPulsar") << "Invalid transmission protocol response length";
		return;
	}

	if (dataLength < 4) {
		ofLogWarning("OrbbecPulsar") << "Transmission protocol data too short: " << dataLength << " bytes";
		return;
	}

	uint32_t modeValue = bytesToUint32(data[9], data[10], data[11], data[12]);

	std::lock_guard<std::mutex> lock(sensorDataMutex);
	switch (modeValue) {
	case 0: transmissionProtocol = "UDP"; break;
	case 1: transmissionProtocol = "TCP"; break;
	default: transmissionProtocol = "unknown"; break;
	}
}

void OrbbecPulsar::parseLidarWarning(const uint8_t* data, int bytesRead) {
	parseString(data, bytesRead, lidarWarning);
}

void OrbbecPulsar::parseSerialNumber(const uint8_t* data, int bytesRead) {
	parseString(data, bytesRead, serialNumber);
}

void OrbbecPulsar::parseFirmwareVersion(const uint8_t* data, int bytesRead) {
	parseString(data, bytesRead, firmwareVersion);
}

void OrbbecPulsar::parseDeviceModel(const uint8_t* data, int bytesRead) {
	parseString(data, bytesRead, model);
}

void OrbbecPulsar::parseWorkingMode(const uint8_t* data, int bytesRead) {
	uint16_t dataLength = bytesToUint16(data[3], data[4]);

	if (dataLength == 0 || bytesRead < (9 + dataLength)) {
		ofLogWarning("OrbbecPulsar") << "Invalid working mode response length";
		return;
	}

	if (dataLength < 4) {
		ofLogWarning("OrbbecPulsar") << "Working mode data too short: " << dataLength << " bytes";
		return;
	}

	uint32_t modeValue = bytesToUint32(data[9], data[10], data[11], data[12]);

	std::lock_guard<std::mutex> lock(sensorDataMutex);
	switch (modeValue) {
	case 0: workingMode = "ranging"; break;
	case 1: workingMode = "standby"; break;
	default: workingMode = "unknown"; break;
	}
}

void OrbbecPulsar::parseSpecialWorkingMode(const uint8_t* data, int bytesRead) {
	uint16_t dataLength = bytesToUint16(data[3], data[4]);

	if (dataLength == 0 || bytesRead < (9 + dataLength)) {
		ofLogWarning("OrbbecPulsar") << "Invalid special working mode response length";
		return;
	}

	if (dataLength < 4) {
		ofLogWarning("OrbbecPulsar") << "Special working mode data too short: " << dataLength << " bytes";
		return;
	}

	uint32_t modeValue = bytesToUint32(data[9], data[10], data[11], data[12]);

	std::lock_guard<std::mutex> lock(sensorDataMutex);
	switch (modeValue) {
	case 0: specialWorkingMode = "normal"; break;
	case 1: specialWorkingMode = "fog"; break;
	default: specialWorkingMode = "unknown"; break;
	}
}

void OrbbecPulsar::parseMotorSpeed(const uint8_t* data, int bytesRead) {
	uint16_t dataLength = bytesToUint16(data[3], data[4]);

	if (dataLength == 0 || bytesRead < (9 + dataLength)) {
		ofLogWarning("OrbbecPulsar") << "Invalid motor speed response length";
		return;
	}

	if (dataLength < 4) {
		ofLogWarning("OrbbecPulsar") << "Motor speed data too short: " << dataLength << " bytes";
		return;
	}

	uint32_t speedValue = bytesToUint32(data[9], data[10], data[11], data[12]);

	std::lock_guard<std::mutex> lock(sensorDataMutex);
	motorSpeed = speedValue;
}

void OrbbecPulsar::parsePointCloudData(const uint8_t* data, int length) {
	uint16_t frameLength = bytesToUint16(data[6], data[7]);
	if (length < frameLength) return;

	uint16_t startAngle = bytesToUint16(data[8], data[9]);
	uint16_t endAngle = bytesToUint16(data[10], data[11]);
	uint16_t angularRes = bytesToUint16(data[12], data[13]);

	uint8_t infoType = data[14];
	uint8_t blockNumber = data[15];
	uint16_t blockSequence = bytesToUint16(data[16], data[17]);

	{
		std::lock_guard<std::mutex> lock(sensorDataMutex);
		timestamp = bytesToUint32(data[18], data[19], data[20], data[21]);

		int lidarStateInt = data[24];
		switch (lidarStateInt) {
		case 0: lidarState = "initial"; break;
		case 1: lidarState = "normal"; break;
		case 2: lidarState = "warning"; break;
		case 3: lidarState = "error"; break;
		default: break;
		}

		// bytes 25-26: fault code (Appendix I of SL450 datasheet)
		uint16_t faultCode = bytesToUint16(data[25], data[26]);
		lidarFaultCode = faultCode;
		lidarFaultDescription = (faultCode != 0) ? faultCodeToString(faultCode) : "";

		// track consecutive error/warning frames for reactive recovery
		if (lidarStateInt == 3) {
			consecutiveErrorFrames++;
			consecutiveWarningFrames = 0;
		}
		else if (lidarStateInt == 2) {
			consecutiveWarningFrames++;
			consecutiveErrorFrames = 0;
		}
		else {
			consecutiveErrorFrames = 0;
			consecutiveWarningFrames = 0;
		}

		uint16_t tempRaw = bytesToUint16(data[31], data[32]);
		temperature = tempRaw * 0.01f;

		currentRotationSpeed = bytesToUint16(data[33], data[34]);
	}

	int pointCount;
	switch (infoType) {
	case 1: pointCount = 200; break;
	case 2: pointCount = 150; break;
	case 3: pointCount = 120; break;
	case 4: pointCount = 100; break;
	case 5: pointCount = 75; break;
	default: pointCount = 200; break;
	}

	extractPointCloudPoints(data + 40, pointCount, startAngle, angularRes * 0.001f);
}

void OrbbecPulsar::extractPointCloudPoints(const uint8_t* data, int pointCount, int startAngle, float angularRes) {
	for (int i = 0; i < pointCount; i++) {
		int dataIndex = i * 4;

		uint16_t distanceRaw = bytesToUint16(data[dataIndex], data[dataIndex + 1]);
		float distance = distanceRaw * 2.0f;

		uint16_t intensity = bytesToUint16(data[dataIndex + 2], data[dataIndex + 3]);
		float pointAngle = (startAngle * 0.01f) + (i * angularRes);

		int newAngularResolution = round(360.0 / angularRes);
		if (newAngularResolution != angularResolution) {
			angularResolution = newAngularResolution;
			initializeVectors();
		}

		int coordIndex = (int)((pointAngle - 45.0f) / 360.0f * angularResolution);

		if (coordIndex >= 0 && coordIndex < angularResolution) {
			if (distance >= 0) {
				std::lock_guard<std::mutex> lock(distancesMutex);
				distances[coordIndex] = distance;
			}
		}
	}

	std::lock_guard<std::mutex> lock(distancesAvailableMutex);
	newDistancesAvailable = true;
}

uint8_t OrbbecPulsar::calculateCRC8(const vector<uint8_t>& data) {
	static const uint8_t CrcTable[256] = {
		0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3, 0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11, 0x5c, 0xa9, 0xe4, 0x33, 0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8, 0xf5, 0x1f, 0x52, 0x85, 0xc8, 0x66, 0x2b, 0xfc, 0xb1, 0xed, 0xa0, 0x77, 0x3a, 0x94, 0xd9, 0x0e, 0x43, 0xb6, 0xfb, 0x2c, 0x61, 0xcf, 0x82, 0x55, 0x18, 0x44, 0x09, 0xde, 0x93, 0x3d, 0x70, 0xa7, 0xea, 0x3e, 0x73, 0xa4, 0xe9, 0x47, 0x0a, 0xdd, 0x90, 0xcc, 0x81, 0x56, 0x1b, 0xb5, 0xf8, 0x2f, 0x62, 0x97, 0xda, 0x0d, 0x40, 0xee, 0xa3, 0x74, 0x39, 0x65, 0x28, 0xff, 0xb2, 0x1c, 0x51, 0x86, 0xcb, 0x21, 0x6c, 0xbb, 0xf6, 0x58, 0x15, 0xc2, 0x8f, 0xd3, 0x9e, 0x49, 0x04, 0xaa, 0xe7, 0x30, 0x7d, 0x88, 0xc5, 0x12, 0x5f, 0xf1, 0xbc, 0x6b, 0x26, 0x7a, 0x37, 0xe0, 0xad, 0x03, 0x4e, 0x99, 0xd4, 0x7c, 0x31, 0xe6, 0xab, 0x05, 0x48, 0x9f, 0xd2, 0x8e, 0xc3, 0x14, 0x59, 0xf7, 0xba, 0x6d, 0x20, 0xd5, 0x98, 0x4f, 0x02, 0xac, 0xe1, 0x36, 0x7b, 0x27, 0x6a, 0xbd, 0xf0, 0x5e, 0x13, 0xc4, 0x89, 0x63, 0x2e, 0xf9, 0xb4, 0x1a, 0x57, 0x80, 0xcd, 0x91, 0xdc, 0x0b, 0x46, 0xe8, 0xa5, 0x72, 0x3f, 0xca, 0x87, 0x50, 0x1d, 0xb3, 0xfe, 0x29, 0x64, 0x38, 0x75, 0xa2, 0xef, 0x41, 0x0c, 0xdb, 0x96, 0x42, 0x0f, 0xd8, 0x95, 0x3b, 0x76, 0xa1, 0xec, 0xb0, 0xfd, 0x2a, 0x67, 0xc9, 0x84, 0x53, 0x1e, 0xeb, 0xa6, 0x71, 0x3c, 0x92, 0xdf, 0x08, 0x45, 0x19, 0x54, 0x83, 0xce, 0x60, 0x2d, 0xfa, 0xb7, 0x5d, 0x10, 0xc7, 0x8a, 0x24, 0x69, 0xbe, 0xf3, 0xaf, 0xe2, 0x35, 0x78, 0xd6, 0x9b, 0x4c, 0x01, 0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17, 0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8
	};

	uint8_t crc = 0;
	for (uint8_t byte : data) {
		crc = CrcTable[(crc ^ byte) & 0xff];
	}
	return crc;
}

uint8_t OrbbecPulsar::calculateCRC8Fast(const uint8_t* data, size_t length) {
	static const uint8_t CrcTable[256] = {
		0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3, 0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11, 0x5c, 0xa9, 0xe4, 0x33, 0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8, 0xf5, 0x1f, 0x52, 0x85, 0xc8, 0x66, 0x2b, 0xfc, 0xb1, 0xed, 0xa0, 0x77, 0x3a, 0x94, 0xd9, 0x0e, 0x43, 0xb6, 0xfb, 0x2c, 0x61, 0xcf, 0x82, 0x55, 0x18, 0x44, 0x09, 0xde, 0x93, 0x3d, 0x70, 0xa7, 0xea, 0x3e, 0x73, 0xa4, 0xe9, 0x47, 0x0a, 0xdd, 0x90, 0xcc, 0x81, 0x56, 0x1b, 0xb5, 0xf8, 0x2f, 0x62, 0x97, 0xda, 0x0d, 0x40, 0xee, 0xa3, 0x74, 0x39, 0x65, 0x28, 0xff, 0xb2, 0x1c, 0x51, 0x86, 0xcb, 0x21, 0x6c, 0xbb, 0xf6, 0x58, 0x15, 0xc2, 0x8f, 0xd3, 0x9e, 0x49, 0x04, 0xaa, 0xe7, 0x30, 0x7d, 0x88, 0xc5, 0x12, 0x5f, 0xf1, 0xbc, 0x6b, 0x26, 0x7a, 0x37, 0xe0, 0xad, 0x03, 0x4e, 0x99, 0xd4, 0x7c, 0x31, 0xe6, 0xab, 0x05, 0x48, 0x9f, 0xd2, 0x8e, 0xc3, 0x14, 0x59, 0xf7, 0xba, 0x6d, 0x20, 0xd5, 0x98, 0x4f, 0x02, 0xac, 0xe1, 0x36, 0x7b, 0x27, 0x6a, 0xbd, 0xf0, 0x5e, 0x13, 0xc4, 0x89, 0x63, 0x2e, 0xf9, 0xb4, 0x1a, 0x57, 0x80, 0xcd, 0x91, 0xdc, 0x0b, 0x46, 0xe8, 0xa5, 0x72, 0x3f, 0xca, 0x87, 0x50, 0x1d, 0xb3, 0xfe, 0x29, 0x64, 0x38, 0x75, 0xa2, 0xef, 0x41, 0x0c, 0xdb, 0x96, 0x42, 0x0f, 0xd8, 0x95, 0x3b, 0x76, 0xa1, 0xec, 0xb0, 0xfd, 0x2a, 0x67, 0xc9, 0x84, 0x53, 0x1e, 0xeb, 0xa6, 0x71, 0x3c, 0x92, 0xdf, 0x08, 0x45, 0x19, 0x54, 0x83, 0xce, 0x60, 0x2d, 0xfa, 0xb7, 0x5d, 0x10, 0xc7, 0x8a, 0x24, 0x69, 0xbe, 0xf3, 0xaf, 0xe2, 0x35, 0x78, 0xd6, 0x9b, 0x4c, 0x01, 0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17, 0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8
	};

	uint8_t crc = 0;
	for (size_t i = 0; i < length; ++i) {
		crc = CrcTable[(crc ^ data[i]) & 0xff];
	}
	return crc;
}

uint16_t OrbbecPulsar::bytesToUint16(uint8_t high, uint8_t low) {
	return (high << 8) | low;
}

uint32_t OrbbecPulsar::bytesToUint32(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0) {
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

// -----------------------------------------------------------------------------
// Fault code lookup — Appendix I of SL450 datasheet
// -----------------------------------------------------------------------------

std::string OrbbecPulsar::faultCodeToString(uint16_t code) {
	switch (code) {
	case 0x0101: return "APD over-temperature";
	case 0x0102: return "APD under-temperature";
	case 0x0103: return "LD over-temperature";
	case 0x0104: return "LD under-temperature";
	case 0x0105: return "APD voltage anomaly";
	case 0x0106: return "High power TX voltage anomaly";
	case 0x0107: return "Low power TX voltage anomaly";
	case 0x0201: return "MCU over-temperature";
	case 0x0202: return "MCU under-temperature";
	case 0x0203: return "FPGA over-temperature";
	case 0x0204: return "FPGA under-temperature";
	case 0x0205: return "Overvoltage";
	case 0x0206: return "Undervoltage";
	case 0x0207: return "Overcurrent";
	case 0x0208: return "Undercurrent";
	case 0x0301: return "Motor over-speed";
	case 0x0302: return "Motor under-speed";
	case 0x0303: return "Motor blockage";
	case 0x0304: return "Encoder signal anomaly";
	case 0x0305: return "Code tooth signal anomaly";
	case 0x0306: return "Point cloud data anomaly";
	case 0x0307: return "Ranging anomaly";
	case 0x0308: return "FPGA/MCU handshake failure (startup)";
	case 0x0309: return "FPGA/MCU handshake failure (operation)";
	case 0x030A: return "FPGA initialization anomaly";
	case 0x030B: return "FPGA/MCU version mismatch";
	case 0x030C: return "Dirt level 1";
	case 0x030D: return "Dirt level 2";
	case 0x030E: return "Dirt level 3";
	case 0x030F: return "Dirt level 4";
	case 0x0310: return "Dirt level 5";
	case 0x0311: return "Dirt level 6";
	case 0x0312: return "Dirt level 7";
	case 0x0313: return "Dirt level 8";
	case 0x0314: return "Dirt level 9";
	case 0x0315: return "Dirt level 10";
	case 0x0316: return "MCU clock frequency anomaly";
	case 0x0317: return "MCU input voltage anomaly";
	case 0x0318: return "LiDAR internal packet loss";
	case 0x0401: return "Code density init anomaly";
	case 0x0402: return "Ranging module init failure";
	case 0x0403: return "Code tooth/encoder calibration anomaly";
	case 0x0404: return "ITF board init anomaly";
	case 0x0405: return "e2prom init read anomaly";
	case 0x0406: return "Serial communication failure";
	case 0x0407: return "FPGA detects motor blockage";
	case 0x0408: return "FPGA detects encoder glitches";
	case 0x0409: return "FPGA status anomaly";
	case 0x040A: return "FPGA point cloud anomaly";
	default: {
		char buf[16];
		snprintf(buf, sizeof(buf), "unknown (0x%04X)", code);
		return std::string(buf);
	}
	}
}