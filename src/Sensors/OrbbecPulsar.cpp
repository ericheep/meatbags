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
    
    initializeVectors();
    setupParameters();
    
    statusTimer = 0.0;
    statusTimeInterval = 0.2;
    
    checkTimer = 0.0;
    checkTimeInterval = 0.5;
    
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
    
    stopThread();
    waitForThread(true);
}

OrbbecPulsar::~OrbbecPulsar() {
    sendDisableDataStreamCommand();
    sleep(500);
}

void OrbbecPulsar::threadedFunction() {
    this_thread::sleep_for(chrono::milliseconds(200));
    
    bool tcpConnected = tcpSetup();
    this_thread::sleep_for(chrono::milliseconds(300));
    
    if (tcpConnected) {
        sendConnectCommand();
    }
    
    while (isThreadRunning() && tcpConnected) {
        uint8_t data[1024];
        int bytesRead = tcpClient.receiveRawBytes((char*)data, sizeof(data));
        
        if (bytesRead > 0) {
            if (isControlResponse(data, bytesRead)) {
                parseControlResponse(data, bytesRead);
            } else if (isPointCloudData(data, bytesRead)){
                parsePointCloudData(data, bytesRead);
            }
        }
        
        threadInactiveTimer.store(0.0);;
        this_thread::sleep_for(chrono::milliseconds(1));
    }
}

bool OrbbecPulsar::isPointCloudData(const uint8_t* data,  int bytesRead) {
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
    double currentValue = threadInactiveTimer.load();
    threadInactiveTimer.store(currentValue + lastFrameTime);
    checkTimer += lastFrameTime;
    statusTimer += lastFrameTime;
    
    if (statusTimer > statusTimeInterval) {
        sendNextStatusCommand();
        statusTimer = 0;
    }
    
    if (checkTimer > checkTimeInterval) {
        checkNextSetting();
        checkTimer = 0;
    }
    
    updateDistances();
    updateSensorInfo();
    checkIfReconnect();
    checkIfThreadRunning();
}

void OrbbecPulsar::updateSensorInfo() {
    if (!showSensorInformation) return;
    
    sensorInfoLines.clear();
    {
        std::lock_guard<std::mutex> lock(sensorDataMutex);
        sensorInfoLines.push_back("Model: " + model);
        sensorInfoLines.push_back("Firmware: " + firmwareVersion);
        sensorInfoLines.push_back("Serial: " + serialNumber);
        sensorInfoLines.push_back("LiDAR state: " + lidarState);
        sensorInfoLines.push_back("Transmission Protocol: " + transmissionProtocol);
        sensorInfoLines.push_back("Set Motor Speed: " + to_string(motorSpeed) + " RPM");
        sensorInfoLines.push_back("Real Time Motor Speed: " + to_string(currentRotationSpeed) + " RPM");
        sensorInfoLines.push_back("Temperature: " + to_string(temperature) + "°C");
        sensorInfoLines.push_back("IP Address: " + ipAddress.get());
        sensorInfoLines.push_back("Port: " + to_string(port));
        sensorInfoLines.push_back("Connection: " + connectionStatus);
        sensorInfoLines.push_back("Timestamp: " + to_string(timestamp));
        sensorInfoLines.push_back("Working Mode: " + workingMode);
        sensorInfoLines.push_back("Warning: " + lidarWarning);
        sensorInfoLines.push_back("Special Working Mode: " + specialWorkingMode);
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
    uint32_t speedValue;
    switch(guiMotorSpeed) {
        case 1: speedValue = 900; break;   // 900 RPM for 15Hz
        case 2: speedValue = 1200; break;  // 1200 RPM for 20Hz
        case 3: speedValue = 1500; break;  // 1500 RPM for 25Hz
        case 4: speedValue = 1800; break;  // 1800 RPM for 30Hz
        case 5: speedValue = 2400; break;  // 2400 RPM for 40Hz
        default: speedValue = 900; break;   // Default to 15Hz
    }
    
    if (speedValue != motorSpeed) {
        cout << motorSpeed << endl;
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

// control command methods
void OrbbecPulsar::sendConnectCommand() {
    vector<uint8_t> data = {0x12, 0x34, 0x56, 0x78};
    sendControlMessage(REG_CONNECT_DEVICE, data);
}

void OrbbecPulsar::sendEnableDataStreamCommand() {
    vector<uint8_t> data = {0x00, 0x00, 0x00, 0x01};
    sendControlMessage(REG_SET_DATA_STREAM, data);
}

void OrbbecPulsar::sendDisableDataStreamCommand() {
    vector<uint8_t> data = {0x00, 0x00, 0x00, 0x00};
    sendControlMessage(REG_SET_DATA_STREAM, data);
}

void OrbbecPulsar::sendSetStandbyMode() {
    vector<uint8_t> data = {0x00, 0x00, 0x00, 0x01};
    sendControlMessage(REG_SET_WORKING_MODE, data);
}

void OrbbecPulsar::sendSetRangingMode() {
    vector<uint8_t> data = {0x00, 0x00, 0x00, 0x00};
    sendControlMessage(REG_SET_WORKING_MODE, data);
}

void OrbbecPulsar::sendSetNormalModeCommand() {
    vector<uint8_t> data = {0x00, 0x00, 0x00, 0x00};
    sendControlMessage(REG_SET_SPECIAL_WORKING_MODE, data);
}

void OrbbecPulsar::sendSetFogModeCommand() {
    vector<uint8_t> data = {0x00, 0x00, 0x00, 0x01};
    sendControlMessage(REG_SET_SPECIAL_WORKING_MODE, data);
}

void OrbbecPulsar::sendSetMotorSpeedCommand(int speed) {
    switch(speed) {
        case 900: break;
        case 1200: break;
        case 1500: break;
        case 1800: break;
        case 2400: break;
        default: return; break;
    }
    
    vector<uint8_t> data = {
        (uint8_t)((speed >> 24) & 0xFF),
        (uint8_t)((speed >> 16) & 0xFF),
        (uint8_t)((speed >> 8) & 0xFF),
        (uint8_t)(speed & 0xFF)
    };
    
    sendControlMessage(REG_SET_MOTOR_SPEED, data);
}

void OrbbecPulsar::sendSetTCPModeCommand() {
    vector<uint8_t> command = {
        0x01, 0xFE,             // frame header
        0x01,                   // protocol version
        0x00, 0x04,             // data segment length
        0x01, 0x07,             // control code
        0x00, 0x00,             // response code
        0x00, 0x00, 0x00, 0x01  // Data: 0x00000001 for TCP mode
    };
    
    uint8_t crc = calculateCRC8(command);
    command.push_back(crc);
    
    sendControlCommand(command);
}

void OrbbecPulsar::sendControlMessage(uint16_t registerAddr, const vector<uint8_t>& dataSegment) {
    uint16_t dataLength = dataSegment.size();
    size_t totalSize = dataLength + 9 + 1;
    commandBuffer.resize(totalSize);
    
    commandBuffer[0] = 0x01;
    commandBuffer[1] = 0xFE;
    commandBuffer[2] = 0x01;
    commandBuffer[3] = (uint8_t)((dataSegment.size() >> 8) & 0xFF);
    commandBuffer[4] = (uint8_t)(dataSegment.size() & 0xFF);
    commandBuffer[5] = (uint8_t)((registerAddr >> 8) & 0xFF);
    commandBuffer[6] = (uint8_t)(registerAddr & 0xFF);
    commandBuffer[7] = 0x00;
    commandBuffer[8] = 0x00;
    
    // append data segment
    if (!dataSegment.empty()) {
        std::memcpy(commandBuffer.data() + 9, dataSegment.data(), dataSegment.size());
    }
    
    uint8_t crc = calculateCRC8Fast(commandBuffer.data(), totalSize - 1);
    commandBuffer[totalSize - 1] = crc;
    
    sendControlCommand(commandBuffer);
}

void OrbbecPulsar::sendControlCommand(const vector<uint8_t>& command) {
    if (tcpClient.isConnected()) {
        tcpClient.sendRawBytes((char*)command.data(), command.size());
    }
}

void OrbbecPulsar::parseControlResponse(const uint8_t* data,  int bytesRead) {
    int16_t controlCode = bytesToUint16(data[5], data[6]);
    int16_t responseCode = bytesToUint16(data[7], data[8]);
    
    bool success = (responseCode == 0x0100);
    
    if (success) {
        switch(controlCode) {
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
            default:  break;
        }
    }
}

string OrbbecPulsar::parseString(const uint8_t* data, int bytesRead) {
    uint16_t dataLength = bytesToUint16(data[3], data[4]);
    
    if (dataLength == 0 || bytesRead < (9 + dataLength)) {
        ofLogWarning("OrbbecPulsar") << "Invalid response length";
        return "NULL";
    }
    
    string str = "";
    for (int i = 9; i < 9 + dataLength && i < bytesRead; i++) {
        if (data[i] != 0) {
            str += char(data[i]);
        }
    }
    
    str.erase(std::find_if(str.rbegin(), str.rend(),
                           [](unsigned char ch) { return !std::isspace(ch); }).base(),
              str.end());
    
    return str;
}

void OrbbecPulsar::parseTransmissionProtocol(const uint8_t* data, int bytesRead) {
    uint16_t dataLength = bytesToUint16(data[3], data[4]);
    
    if (dataLength == 0 || bytesRead < (9 + dataLength)) {
        ofLogWarning("OrbbecPulsar") << "Invalid motor speed response length";
        return;
    }
    
    if (dataLength < 4) {
        ofLogWarning("OrbbecPulsar") << "Motor speed data too short: " << dataLength << " bytes";
        return;
    }
    
    uint32_t modeValue = bytesToUint32(data[9], data[10], data[11], data[12]);
    
    std::lock_guard<std::mutex> lock(sensorDataMutex);
    switch (modeValue) {
        case 0:
            transmissionProtocol = "UDP";
            break;
        case 1:
            transmissionProtocol = "TCP";
            break;
        default:
            transmissionProtocol = "unknown";
            break;
    }
}

void OrbbecPulsar::parseLidarWarning(const uint8_t* data, int bytesRead) {
    lidarWarning = parseString(data, bytesRead);
}

void OrbbecPulsar::parseSerialNumber(const uint8_t* data, int bytesRead) {
    serialNumber = parseString(data, bytesRead);
}

void OrbbecPulsar::parseFirmwareVersion(const uint8_t* data, int bytesRead) {
    firmwareVersion = parseString(data, bytesRead);
}

void OrbbecPulsar::parseDeviceModel(const uint8_t* data, int bytesRead) {
    model = parseString(data, bytesRead);
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
    switch (modeValue) {
        case 0:
            workingMode = "ranging";
            break;
        case 1:
            workingMode = "standby";
            break;
        default:
            workingMode = "unknown";
            break;
    }
}

void OrbbecPulsar::parseSpecialWorkingMode(const uint8_t* data, int bytesRead) {
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
    
    {
        std::lock_guard<std::mutex> lock(sensorDataMutex);
        switch (modeValue) {
            case 0:
                specialWorkingMode = "normal";
                break;
            case 1:
                specialWorkingMode = "fog";
                break;
            default:
                specialWorkingMode = "unknown";
                break;
        }
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
    
    motorSpeed = speedValue;
}

void OrbbecPulsar::parsePointCloudData(const uint8_t* data, int length) {
    uint16_t frameLength = bytesToUint16(data[6], data[7]);
    if (length < frameLength) return; // incomplete frame
    
    // Extract angle information (bytes 8-13)
    uint16_t startAngle = bytesToUint16(data[8], data[9]);    // unit: 0.01°
    uint16_t endAngle = bytesToUint16(data[10], data[11]);    // unit: 0.01°
    uint16_t angularRes = bytesToUint16(data[12], data[13]);  // unit: 0.001°
    
    uint8_t infoType = data[14];      // 1=15Hz, 2=20Hz, 3=25Hz, 4=30Hz, 5=40Hz
    uint8_t blockNumber = data[15];   // 1-18
    uint16_t blockSequence = bytesToUint16(data[16], data[17]);
    
    // extract timestamp (bytes 18-21)
    timestamp = bytesToUint32(data[18], data[19], data[20], data[21]);
    
    // get lidar state
    int lidarStateInt = data[24];
    switch(lidarStateInt) {
        case 0: lidarState = "initial"; break;
        case 1: lidarState = "normal"; break;
        case 2: lidarState = "warning"; break;
        case 3: lidarState = "error"; break;
        default: break;
    }
    
    // extract temperature (bytes 31-32)
    uint16_t tempRaw = bytesToUint16(data[31], data[32]);
    temperature = tempRaw * 0.01f; // Unit: 0.01°C
    
    // extract rotation speed (bytes 33-34)
    currentRotationSpeed = bytesToUint16(data[33], data[34]);
    
    // determine point count based on info type
    int pointCount;
    switch(infoType) {
        case 1: pointCount = 200; break; // 15Hz
        case 2: pointCount = 150; break; // 20Hz
        case 3: pointCount = 120; break; // 25Hz
        case 4: pointCount = 100; break; // 30Hz
        case 5: pointCount = 75; break;  // 40Hz
        default: pointCount = 200; break;
    }
    
    // extract point cloud data (starts at byte 40)
    extractPointCloudPoints(data + 40, pointCount, startAngle, angularRes * 0.001f);
}

void OrbbecPulsar::extractPointCloudPoints(const uint8_t* data, int pointCount, int startAngle, float angularRes) {
    for (int i = 0; i < pointCount; i++) {
        // each point is 4 bytes: 2 bytes distance + 2 bytes intensity
        int dataIndex = i * 4;
        
        uint16_t distanceRaw = bytesToUint16(data[dataIndex], data[dataIndex + 1]);
        float distance = distanceRaw * 2.0f; // convert to mm
        
        uint16_t intensity = bytesToUint16(data[dataIndex + 2], data[dataIndex + 3]);
        float pointAngle = (startAngle * 0.01f) + (i * angularRes); // convert to degrees
        
        int newAngularResolution = round(360.0 / angularRes);
        if (newAngularResolution != angularResolution) {
            angularResolution = newAngularResolution;
            initializeVectors();
        }
        
        // convert to coordinate index (map 270° range to our angular resolution)
        int coordIndex = (int)((pointAngle - 45.0f) / 360.0f * angularResolution);
        
        if (coordIndex >= 0 && coordIndex < angularResolution) {
            if (distance >= 0) {
                std::lock_guard<std::mutex> lock(distancesMutex);
                distances[coordIndex] = distance;
            }
            
            // if (coordIndex < intensities.size()) {
            //     intensities[coordIndex] = intensity;
            // }
        }
    }
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
