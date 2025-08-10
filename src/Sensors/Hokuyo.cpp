//
//  Hokuyo.cpp
//  meatbags

#include "Hokuyo.hpp"

Hokuyo::Hokuyo() {
    // Hokuyo specific settings, 180 degrees, front facing range
    angularResolution = 1440;
    startStep = 0;
    endStep = 1079;
    
    port = 10940;
    
    localIPAddress = "0.0.0.0";
    interface = "";
    
    callIntensitiesActive = true;
    
    initializeVectors();
    setupParameters();
        
    showSensorInformation = false;
}

Hokuyo::~Hokuyo() {
    stopThread();
}

void Hokuyo::shutdown() {
    ofLogNotice("Hokuyo") << "Shutting down Hokuyo sensor " << index;
    
    // Call base class shutdown first
    Sensor::shutdown();
    
    // Stop data stream before closing connection
    if (tcpClient.isConnected()) {
        ofSleepMillis(50); // Give time for command to be sent
    }
    
    // Close TCP connection
    tcpClient.close();
    
    // Stop the thread cleanly
    if (isThreadRunning()) {
        stopThread();
        waitForThread(true); // Wait for thread to actually stop
    }
    
    // Reset connection state
    isConnected = false;
    connectionStatus = "Disconnected";
    
    // Reset timers
    threadInactiveTimer = 0.0;
    reconnectionTimer = 0.0;
    statusTimer = 0.0;
    checkTimer = 0.0;
    
    ofLogNotice("Hokuyo") << "Hokuyo sensor " << index << " shutdown complete";
}

void Hokuyo::update() {    
    threadInactiveTimer += lastFrameTime;
    if (threadInactiveTimer > threadInactiveTimeInterval) {
        startThread();
        threadInactiveTimer = 0;
    }
        
    if(tcpClient.isConnected()) {
        if (!isConnected) {
            connectionStatus = "CONNECTED";
            
            status = "Connected at " + tcpClient.getIP() + " " + to_string(tcpClient.getPort());
            isConnected = true;
            sendStatusInfoCommand();
            sendVersionInfoCommand();
        }
        
        checkStatus();
    } else {
        for (int i = 0; i < angularResolution; i++) {
            coordinates[i] = ofPoint::zero();
            intensities[i] = 0;
        }
        
        if (isConnected) {
            connectionStatus = "DISCONNECTED";
            isConnected = false;
        }
        
        if (autoReconnectActive) reconnect();
    }
}

void Hokuyo::connect() {
    if (ipAddress.get() == "0.0.0.0") return;
    
    if (!isThreadRunning()) {
        startThread();
    } else {
        ofLogWarning("Hokuyo") << "Thread already running; ignoring connect() call.";
    }
}

void Hokuyo::reconnect() {
    reconnectionTimer += lastFrameTime;
    
    if (reconnectionTimer > reconnectionTimeInterval){
        connectionStatus = "Attempting to reconnect";
        reconnectionTimer = 0;
        connect();
    }
}

void Hokuyo::close() {
    tcpClient.close();
}

void Hokuyo::setIPAddress(string &ipAddress) {
    tcpClient.close();
    if (isThreadRunning()) {
        stopThread();
    }
    connect();
}

/*void Hokuyo::setSleep(bool& isSleeping) {
    if (isSleeping) {
        sendMeasurementModeOffCommand();
    } else {
        sendMeasurementModeOnCommand();
        sendStreamDistancesCommand();
    }
}*/

void Hokuyo::threadedFunction() {
    tcpClient.setup(ipAddress, port, false, interface, localIPAddress);
    tcpClient.setMessageDelimiter("\012\012");
    sendStreamDistancesCommand();
    
    cout << "THREAD" << endl;
    while(tcpClient.isConnected()) {
        streamingTimer += lastFrameTime;
        
        string response = tcpClient.receive();
        if (response.length() > 0){
            parseResponse(response);
        }
        
        if (streamingTimer > streamingTimeInterval) {
            if (!isSleeping) sendStreamDistancesCommand();
            streamingTimer = 0;
        }
        
        threadInactiveTimer = 0;
        sleep(1);
    }
}

void Hokuyo::sendRebootCommand() {
    // send command twice to register, per spec
    send("RB");
    send("RB");
}

void Hokuyo::sendResetStatusCommand() {
    send("RS");
}

void Hokuyo::sendSetMotorSpeedCommand(int motorSpeed) {
    motorSpeed = ofClamp(motorSpeed, 0, 99);
    
    char motorSpeedChars[3];
    std::snprintf(motorSpeedChars, 3, "%02d", motorSpeed);
    string motorSpeedBytes = (string) motorSpeedChars;
    
    send("CR" + motorSpeedBytes);
}

// not implemented, not working
void Hokuyo::sendSetIPAddressCommand() {
    /*string ipAddress;
    string netmask;
    string gateway;
    
    string ipAddressBytes = formatIpv4String(ipAddress);
    string netmaskBytes = formatIpv4String(netmask);
    string gatewayBytes = formatIpv4String(gateway);
    
    send("$IP" + ipAddress + netmask + gateway);*/
}

void Hokuyo::sendStatusInfoCommand() {
    send("II");
}

void Hokuyo::sendVersionInfoCommand() {
    send("VV");
}

void Hokuyo::sendParameterInfoCommand() {
    send("PP");
}

void Hokuyo::sendMeasurementModeOnCommand() {
    send("BM");
}

void Hokuyo::sendMeasurementModeOffCommand() {
    send("QT");
}

void Hokuyo::sendGetDistancesCommand() {
    string msg = formatDistanceMessage("GD");
    send(msg);
}

void Hokuyo::sendStreamDistancesCommand() {
    string msg = formatStreamDistancesMessage("MD");
    send(msg);
}

void Hokuyo::sendGetDistancesAndIntensitiesCommand() {
    string msg = formatDistanceMessage("GE");
    send(msg);
}

void Hokuyo::send(string msg) {
    tcpClient.send(msg + "\012");
}

string Hokuyo::formatDistanceMessage(string command) {
    string startStepString = zeroPad(startStep, 4);
    string endStepString = zeroPad(endStep, 4);
    string clusterCountString = zeroPad(1, 2);
    
    return command + startStepString + endStepString + clusterCountString;
}

string Hokuyo::formatStreamDistancesMessage(string command) {
    string startStepString = zeroPad(startStep, 4);
    string endStepString = zeroPad(endStep, 4);
    string clusterCountString = zeroPad(1, 2);
    string scanIntervalString = zeroPad(0, 2);
    string numberOfScansString = zeroPad(0, 1);
    
    return command + startStepString + endStepString + clusterCountString + scanIntervalString + numberOfScansString;
}

string Hokuyo::zeroPad(int value, int width) {
    std::ostringstream ss;
    ss << std::setw(width) << std::setfill('0') << value;
    
    return ss.str();
}

string Hokuyo::formatIpv4String(string command) {
    return command;
}

void Hokuyo::checkStatus() {
    statusTimer += lastFrameTime;
    
    if (statusTimer > statusTimeInterval){
        sendStatusInfoCommand();
        sendVersionInfoCommand();
        sendParameterInfoCommand();
        statusTimer = 0.0;
    }
}

void Hokuyo::draw() {
    if (!showSensorInformation) return;

    vector<string> sensorInfoLines;
    sensorInfoLines.push_back("version: " + vendorInfo);
    sensorInfoLines.push_back("model: " + model);
    sensorInfoLines.push_back("firmware: " +  firmwareVersion);
    sensorInfoLines.push_back("protocol: " + protocolVersion);
    sensorInfoLines.push_back("serial: " + serialNumber);
    sensorInfoLines.push_back("laser state: " + laserState);
    sensorInfoLines.push_back("polling start step: " + to_string(startStep));
    sensorInfoLines.push_back("polling end step: " + to_string(endStep));
    sensorInfoLines.push_back("measurement mode: " + measurementMode);
    sensorInfoLines.push_back("bitrate: " + bitRate);
    sensorInfoLines.push_back("timestamp: " + to_string(timeStamp));
    sensorInfoLines.push_back("sensor diagnostic: " + sensorDiagnostic);
    sensorInfoLines.push_back("IP Address: " + tcpClient.getIP());
    sensorInfoLines.push_back("port: " + to_string(port));
    sensorInfoLines.push_back("connection status: " + connectionStatus);
    sensorInfoLines.push_back("status: " + status);
    sensorInfoLines.push_back("min measurable dist: " + minimumMeasurableDistance);
    sensorInfoLines.push_back("max measurable dist: " + maximumMeasureableDistance);
    sensorInfoLines.push_back("angular resolution: " + angularResolutionInfo);
    sensorInfoLines.push_back("starting step: " + startingStep);
    sensorInfoLines.push_back("ending step: " + endingStep);
    sensorInfoLines.push_back("front direction steps: " + stepNumberOfFrontDirection);
    sensorInfoLines.push_back("scanning speed: " + scanningSpeed);
    
    drawSensorInfo(sensorInfoLines);
}

string Hokuyo::checkSum(string str, int fromEnd) {
    if (str.size() <= static_cast<size_t>(fromEnd)) return "";
    
    size_t payloadLen = str.size() - fromEnd;
    int sum = 0;
    
    for (size_t i = 0; i < payloadLen; ++i) {
        sum += static_cast<unsigned char>(str[i]);
    }
    
    int expected = ((sum & 0x3F) + 0x30);
    int received = static_cast<unsigned char>(str.back());
    
    if (expected == received) {
        return str.substr(0, payloadLen);
    } else {
        return "";
    }
}

void Hokuyo::parseResponse(const string& str) {
    vector<string> lines = splitStringByNewline(str);
    if (lines.size() == 0 || lines.empty()) return;
    
    string& header = lines[0];
    string command = header.substr(0, 2);
    
    if (command == "MD") parseStreamingDistances(lines);
    else if (command == "GD") parseDistances(lines);
    else if (command == "GE") parseDistancesAndIntensities(lines);
    else if (command == "BM") parseActivate(lines);
    else if (command == "QT") parseQuiet(lines);
    else if (command == "II") parseStatusInfo(lines);
    else if (command == "VV") parseVersionInfo(lines);
    else if (command == "PP") parseParameterInfo(lines);
    else if (command == "CR") parseMotorSpeed(lines);
}

void Hokuyo::parseStreamingDistances(vector<string> packet) {
    if (packet.size() < 2) return;
    
    const string& header = packet[0];
    int startStep = stoi(header.substr(2, 4));
    int endStep = stoi(header.substr(6, 4));
    int clusterCount = stoi(header.substr(10, 2));
    
    // validate status and timestamp
    if (checkSum(packet[1], 1).empty()) return;
    if (checkSum(packet[2], 1).empty()) return;
    
    const int expectedDataPoints = (endStep - startStep + 1);
    const int expectedChars = expectedDataPoints * 3;
    
    string concatenatedData;
    concatenatedData.reserve(expectedChars);
    
    for (int i = 3; i < packet.size(); i++) {
        string decoded = checkSum(packet[i], 1);
        if (decoded.empty()) continue;
        concatenatedData += decoded;
    }
    
    if (concatenatedData.length() < expectedChars) return;
    const char* raw = concatenatedData.c_str();
    size_t numSteps = endStep - startStep + 1;
    
    int step = startStep;
    for (int i = 0; i < numSteps; i++) {
        int distance = sixBitCharDecode(raw + (i * 3), 3);
        cout << "!" << endl;
        createCoordinate(step, distance);
        step += 1;
    }
    
    newCoordinatesAvailable = true;
    streamingTimeInterval = 0;
}

void Hokuyo::parseDistances(vector<string> packet) {
    if (packet.size() < 2) return;
    
    const string& header = packet[0];
    int startStep = stoi(header.substr(2, 4));
    int endStep = stoi(header.substr(6, 4));
    int clusterCount = stoi(header.substr(10, 2));
    
    // validate status and timestamp
    if (checkSum(packet[1], 1).empty()) return;
    if (checkSum(packet[2], 1).empty()) return;
    
    const int expectedDataPoints = (endStep - startStep + 1);
    const int expectedChars = expectedDataPoints * 3;
    
    string concatenatedData;
    concatenatedData.reserve(expectedChars);

    for (int i = 3; i < packet.size(); i++) {
        string decoded = checkSum(packet[i], 1);
        if (decoded.empty()) continue;
        concatenatedData += decoded;
    }
    
    const char* raw = concatenatedData.c_str();
    size_t numSteps = endStep - startStep + 1;
    
    int step = startStep;
    for (int i = 0; i < numSteps; i++) {
        int distance = sixBitCharDecode(raw + (i * 3), 3);
        createCoordinate(step, distance);
        step += 1;
    }
    
    newCoordinatesAvailable = true;
}

void Hokuyo::parseDistancesAndIntensities(vector<string> packet) {
    if (packet.size() <= 2) return;
    
    int startStep = stoi(packet[0].substr(2, 4));
    int endStep = stoi(packet[0].substr(6, 4));
    int clusterCount = stoi(packet[0].substr(10, 2));
    
    // validate status and timestamp
    if (checkSum(packet[1], 1).empty()) return;
    if (checkSum(packet[2], 1).empty()) return;
    
    const int expectedDataPoints = (endStep - startStep + 1);
    const int expectedChars = expectedDataPoints * 3;
    
    string concatenatedData;
    concatenatedData.reserve(expectedChars);

    for (int i = 3; i < packet.size(); i++) {
        string decoded = checkSum(packet[i], 1);
        if (decoded.empty()) continue;
        concatenatedData += decoded;
    }
    
    int step = startStep;
    
    // will revisit later
    // so far intensities aren't useful for people tracking
    /*for (int i = 0; i < concatenatedData.size(); i+=6) {
     string distanceChars = concatenatedData.substr(i, 3);
     string intensityChars = concatenatedData.substr(i + 3, 3);
     
     int distance = sixBitCharDecode(distanceChars);
     int intensity = sixBitCharDecode(intensityChars);
     
     createCoordinate(step, distance);
     intensities[step] = intensity;
     
     step += 1;
     }*/
    
    newCoordinatesAvailable = true;
}

void Hokuyo::parseStatusInfo(vector<string>& packet) {
    for (size_t i = 1; i < packet.size(); i++) {
        const string& rawLine = packet[i];
        string checkedLine = checkSum(rawLine, 2);
        
        if (checkedLine.empty() || checkedLine.size() <= 5) continue;
        
        string name = checkedLine.substr(0, 4);
        string info = checkedLine.substr(5);
        
        if (name == "MODL") { model = info; }
        else if (name == "LASR") { laserState = info; }
        else if (name == "MESM") { measurementMode = info; }
        else if (name == "SBPS") { bitRate = info; }
        else if (name == "TIME" && info.size() >= 3) {
            timeStamp = sixBitCharDecode(info.c_str(), 3);
        }
        else if (name == "STAT") { sensorDiagnostic = info; }
    }
}

void Hokuyo::parseMotorSpeed(vector<string> packet) {
    if (packet.size() <= 2) return;
    
    for (int i = 1; i < packet.size(); i++) {
        string checkedLine = checkSum(packet[i], 2);
    }
}

void Hokuyo::parseVersionInfo(vector<string> packet) {
    for (int i = 1; i < packet.size(); i++) {
        string checkedLine = checkSum(packet[i], 2);
        
        if (checkedLine.size() > 5) {
            string name = checkedLine.substr(0, 4);
            string info = checkedLine.substr(5);
            
            if (name == "VEND") vendorInfo = info;
            else if (name == "PROD") productInfo = info;
            else if (name == "FIRM") firmwareVersion = info;
            else if (name == "PROT") protocolVersion = info;
            else if (name == "SERI") serialNumber = info;
        }
    }
}

void Hokuyo::parseParameterInfo(vector<string> packet) {
    for (int i = 1; i < packet.size(); i++) {
        string checkedLine = checkSum(packet[i], 2);
        
        if (checkedLine.size() > 5) {
            string name = checkedLine.substr(0, 4);
            string info = checkedLine.substr(5);
            
            if (name == "MODL") model = info;
            else if (name == "DMIN") minimumMeasurableDistance = info;
            else if (name == "DMAX") maximumMeasureableDistance = info;
            else if (name == "ARES") angularResolutionInfo = info;
            else if (name == "AMIN") startingStep = info;
            else if (name == "AMAX") endingStep = info;
            else if (name == "AFRT") stepNumberOfFrontDirection = info;
            else if (name == "SCAN") scanningSpeed = info;
        }
    }
}

void Hokuyo::parseActivate(vector<string> packet) {
    for (int i = 1; i < packet.size(); i++) {
        string checkedLine = checkSum(packet[i], 1);
        if (checkedLine == "00") {
            laserState = "SWITCHING ON";
            status = "Laser command recieved without any Error";
        } else if (checkedLine == "01") {
            status = "Unable to control due to laser malfunction";
        } else if (checkedLine == "02") {
            // laserState = "ON";
            status = "Laser is already on.";
            
        }
    }
}

void Hokuyo::parseQuiet(vector<string> packet) {
    for (int i = 1; i < packet.size(); i++) {
        string checkedLine = checkSum(packet[i], 1);
        if (checkedLine == "00") laserState = "SWITCHING OFF";
    }
}

int Hokuyo::sixBitCharDecode(const char* data, int length) {
    if (length == 2) {
        return ((data[0] - 0x30) << 6) |
        ((data[1] - 0x30));
    } else if (length == 3) {
        return ((data[0] - 0x30) << 12) |
        ((data[1] - 0x30) << 6)  |
        ((data[2] - 0x30));
    } else {
        return 0;
    }
}

vector<string> Hokuyo::splitStringByNewline(const string& str) {
    std::vector<std::string> result;
    size_t start = 0;
    
    while (start < str.length()) {
        size_t end = str.find('\n', start);
        if (end == std::string::npos) {
            result.emplace_back(str.substr(start));
            break;
        } else {
            result.emplace_back(str.substr(start, end - start));
            start = end + 1;
        }
    }
    
    return result;
}
