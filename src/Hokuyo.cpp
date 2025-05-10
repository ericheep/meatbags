//
//  Hokuyo.cpp
//

#include "Hokuyo.hpp"

Hokuyo::Hokuyo() {
    reconnectionTimeout = 5.0;
    reconnectionTimer = 0.0;
    
    statusInterval = 3.0;
    statusTimer = 0.0;
    
    pollingInterval = 1.0 / 30;
    pollingTimer = 0;
    
    lastFrameTime = 0;
    
    isConnected = false;
    laserActive = true;
    
    // 180 degrees, front facing range
    startStep = 0;
    endStep = 1079;
    angularResolution = 1440;
    sensorRotationDeg = 0;
    sensorRotationRad = 0;
    clusterCount = 0;
    
    status = "";
    lastStatus = status;
    connectionStatus = "DISCONNECTED";
    
    port = 10940;
    autoReconnectActive = true;
    ipAddress.addListener(this, &Hokuyo::setIPAddress);
    positionX.addListener(this, &Hokuyo::setPositionX);
    positionY.addListener(this, &Hokuyo::setPositionY);
    mirrorAngles.addListener(this, &Hokuyo::setMirrorAngles);
    sensorRotationDeg.addListener(this, &Hokuyo::setSensorRotation);
    alignSensorButton.addListener(this, &Hokuyo::alignSensor);

    netmask = "255.255.255.0";
    gateway = "192.168.0.1";
    localIPAddress = "0.0.0.0";
    
    // containers
    angles.resize(angularResolution);
    coordinates.resize(angularResolution);
    intensities.resize(angularResolution);
    
    callIntensitiesActive = true;
    newCoordinatesAvailable = false;
    
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    
    position = DraggablePoint();
    position.size = 15;
    position.halfSize = position.size * 0.5;
    nosePosition.size = 12;
    nosePosition.halfSize = nosePosition.size * 0.5;
    noseRadius = position.size + position.halfSize;
    
    position.x = 0.0;
    position.y = 0.0;
    position.isMouseOver = false;
    position.isMouseClicked = false;
    
    mirrorAngles = false;
    alignRequested = false;
    
    font.setMedium();
    font.setSize(12);
}

void Hokuyo::alignSensor() {
    alignRequested = true;
}

void Hokuyo::setSensorRotation(float &_sensorRotationDeg) {
    sensorRotationRad = _sensorRotationDeg / 360.0 * TWO_PI;
}

void Hokuyo::threadedFunction() {
    tcpClient.setup(ipAddress, port, false, localIPAddress);
    tcpClient.setMessageDelimiter("\012\012");
}

void Hokuyo::connect() {
    if (ipAddress.get() == "0.0.0.0") return;

    startThread();
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

void Hokuyo::sendSetIPAddressCommand() {
    string ipAddress;
    string netmask;
    string gateway;
    
    string ipAddressBytes = formatIpv4String(ipAddress);
    string netmaskBytes = formatIpv4String(netmask);
    string gatewayBytes = formatIpv4String(gateway);

    send("$IP" + ipAddress + netmask + gateway);
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

void Hokuyo::sendGetDistancesAndIntensitiesCommand() {
    string msg = formatDistanceMessage("GE");
    send(msg);
}

void Hokuyo::send(string msg) {
    tcpClient.send(msg + "\012");
}

string Hokuyo::formatDistanceMessage(string command) {
    char startStepChars[5];
    std::snprintf(startStepChars, 5, "%04d", startStep);
    
    char endStepChars[5];
    std::snprintf(endStepChars, 5, "%04d", endStep);
    
    char clusterCountChars[3];
    std::snprintf(clusterCountChars, 3, "%02d", clusterCount);
    
    return command + (string)startStepChars + (string)endStepChars + (string)clusterCountChars;
}

string Hokuyo::formatIpv4String(string command) {
    
    return command;
}

void Hokuyo::checkStatus() {
    statusTimer += lastFrameTime;

    if (statusTimer > statusInterval){
        sendStatusInfoCommand();
        sendVersionInfoCommand();
        sendParameterInfoCommand();
        statusTimer = 0.0;
    }
    
    if (laserActive && laserState == "OFF") {
        sendMeasurementModeOnCommand();
    }
    
    if (!laserActive && laserState == "ON") {
        sendMeasurementModeOffCommand();
    }
}

void Hokuyo::update() {
    lastFrameTime = ofGetLastFrameTime();
    
    if(tcpClient.isConnected()) {
        if (!isConnected) {
            connectionStatus = "CONNECTED";

            status = "Connected at " + tcpClient.getIP() + " " + to_string(tcpClient.getPort());
            isConnected = true;
            sendStatusInfoCommand();
            sendVersionInfoCommand();
        }

        string response = tcpClient.receive();
        if (response.length() > 0){
            parseResponse(response);
        }
        
        checkStatus();
        
        if (laserState == "ON") {
            pollingTimer += lastFrameTime;
            if (pollingTimer > pollingInterval) {
                if (callIntensitiesActive) {
                    sendGetDistancesAndIntensitiesCommand();
                } else {
                    sendGetDistancesCommand();
                }
                pollingTimer = 0;
            }
        }
    } else {
        for (int i = 0; i < angularResolution; i++) {
            coordinates[i] = ofPoint::zero();
            intensities[i] = 0;
        }
        
        if (isConnected) {
            connectionStatus = "DISCONNECTED";

            status = "Disconnected";
            isConnected = false;
        }
        
        if (autoReconnectActive) reconnect();
    }
    
    if (status != lastStatus) {
        lastStatus = status;
    }
}

// event listeners
void Hokuyo::setLocalIPAddress(string & _localIPAddress) {
    localIPAddress = _localIPAddress;
}

// event listeners
void Hokuyo::setIPAddress(string &ipAddress) {
    tcpClient.close();
    connect();
}

void Hokuyo::setPositionX(float &positionX) {
    position.x = positionX * 1000.0;
}

void Hokuyo::setPositionY(float &positionY) {
    position.y = positionY * 1000.0;
}

void Hokuyo::setInfoPosition(float _x, float _y) {
    x = _x;
    y = _y;
}

void Hokuyo::setMirrorAngles(bool &_mirrorAngles) {
    mirrorAngles = _mirrorAngles;

    // -45 degrees offset
    float thetaOffset = -HALF_PI * 0.5;
    
    for (int i = 0; i < angularResolution; i++) {
        int index = i;
        float theta = (float) index / angularResolution * TWO_PI;

        if (mirrorAngles) {
            index = angularResolution - 1;
            thetaOffset = -HALF_PI * 0.5 - HALF_PI;
            theta *= -1.0;
        }
        
        angles[i] = theta + thetaOffset;
    }
}

void Hokuyo::draw() {
    ofSetColor(ofColor::grey);
    
    
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
    
    float offset = 10;
    
    float textBoxHeight = sensorInfoLines.size() * 16;
    float textBoxWidth = 340;
    
    float textX = x - textBoxWidth * 0.5;
    float textY = y - textBoxHeight * 0.5;
    
    ofFill();
    ofSetColor(0, 0, 0, 300);
    ofRectangle textBox;
    textBox.setFromCenter(x, y, textBoxWidth, textBoxHeight + 8);
    ofDrawRectangle(textBox);
    ofSetColor(ofColor::pink);
    ofNoFill();
    ofDrawRectangle(textBox);
    ofFill();
    
    for (int i = 0; i < sensorInfoLines.size(); i++) {
        float y = textY + 16 * i;
        font.draw(sensorInfoLines[i], textX + 6, y + 10);
    }
}

string Hokuyo::checkSum(string str, int fromEnd) {
    string msg = str.substr(0, str.size() - fromEnd);
    int sum = str.back();
    
    int checkSum = 0;
    for (int i = 0 ; i < msg.length(); i++) {
        int decimal = msg[i];
        checkSum += decimal;
    }
    
    checkSum = (checkSum & 0x3f) + 0x30;
    if (checkSum == sum) {
        return msg;
    } else {
        return "";
    }
}

void Hokuyo::parseResponse(string str) {
    vector<string> lines = splitStringByNewline(str);
    if (lines.size() == 0) return;
    
    string header = lines[0];
    string command = header.substr(0, 2);
            
    if (command == "GD") parseDistances(lines);
    if (command == "GE") parseDistancesAndIntensities(lines);
    if (command == "BM") parseActivate(lines);
    if (command == "QT") parseQuiet(lines);
    if (command == "II") parseStatusInfo(lines);
    if (command == "VV") parseVersionInfo(lines);
    if (command == "PP") parseParameterInfo(lines);
    if (command == "CR") parseMotorSpeed(lines);
}

void Hokuyo::parseDistances(vector<string> packet) {
    if (packet.size() <= 2) return;
    
    int startStep = stoi(packet[0].substr(2, 4));
    int endStep = stoi(packet[0].substr(6, 4));
    int clusterCount = stoi(packet[0].substr(10, 2));
    
    string status = checkSum(packet[1], 1);
    int timeStamp = char2int6bitDecode(checkSum(packet[2], 1));
    
    string concatenatedData = "";
    for (int i = 3; i < packet.size(); i++) {
        string encodedData = checkSum(packet[i], 1);
        concatenatedData += encodedData;
    }
    
    if (concatenatedData.size() % 6 != 0) return;
    if (concatenatedData.size() / 6 != (endStep - startStep) + 1) return;
    
    int step = startStep;
    for (int i = 0; i < concatenatedData.size(); i+=3) {
        string threeChars = concatenatedData.substr(i, 3);
        int distance = char2int6bitDecode(threeChars);
        
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
    
    string status = checkSum(packet[1], 1);
    int timeStamp = char2int6bitDecode(checkSum(packet[2], 1));
    
    string concatenatedData = "";
    for (int i = 3; i < packet.size(); i++) {
        string encodedData = checkSum(packet[i], 1);
        concatenatedData += encodedData;
    }
    
    if (concatenatedData.size() % 6 != 0) return;
    if (concatenatedData.size() / 6 != (endStep - startStep) + 1) return;
    
    int step = startStep;
    for (int i = 0; i < concatenatedData.size(); i+=6) {
        string distanceChars = concatenatedData.substr(i, 3);
        string intensityChars = concatenatedData.substr(i + 3, 3);
        
        int distance = char2int6bitDecode(distanceChars);
        int intensity = char2int6bitDecode(intensityChars);
        
        createCoordinate(step, distance);
        intensities[step] = intensity;
        
        step += 1;
    }
    
    newCoordinatesAvailable = true;
}

void Hokuyo::createCoordinate(int index, float distance) {
    float theta = angles[index] + sensorRotationRad;
    
    float x = cos(theta) * distance;
    float y = sin(theta) * distance;

    coordinates[index].set(ofPoint(x, y) + ofPoint(position.x, position.y));
}

void Hokuyo::parseStatusInfo(vector<string> packet) {
    for (int i = 1; i < packet.size(); i++) {
        string checkedLine = checkSum(packet[i], 2);
        
        if (checkedLine.size() > 5) {
            string name = checkedLine.substr(0, 4);
            string info = checkedLine.substr(5);
            
            if (name == "MODL") model = info;
            if (name == "LASR") laserState = info;
            if (name == "MESM") measurementMode = info;
            if (name == "SBPS") bitRate = info;
            if (name == "TIME") timeStamp = char2int6bitDecode(info);
            if (name == "STAT") sensorDiagnostic = info;
        }
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
            if (name == "PROD") productInfo = info;
            if (name == "FIRM") firmwareVersion = info;
            if (name == "PROT") protocolVersion = info;
            if (name == "SERI") serialNumber = info;
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
            if (name == "DMIN") minimumMeasurableDistance = info;
            if (name == "DMAX") maximumMeasureableDistance = info;
            if (name == "ARES") angularResolutionInfo = info;
            if (name == "AMIN") startingStep = info;
            if (name == "AMAX") endingStep = info;
            if (name == "AFRT") stepNumberOfFrontDirection = info;
            if (name == "SCAN") scanningSpeed = info;
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
            laserState = "ON";
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

int Hokuyo::char2int6bitDecode(string str) {
    int sum = 0;
    for (int i = 0; i < str.size(); i++) {
        int decimal = str[i];
        decimal -= 0x30;
        decimal = decimal << (6 * (str.size() - i - 1));
        sum += decimal;
    }
    
    return sum;
}

void Hokuyo::reconnect() {
    reconnectionTimer += lastFrameTime;
    
    if (reconnectionTimer > reconnectionTimeout){
        status = "Attempting to reconnect";
        reconnectionTimer = 0;
        connect();
    }
}

void Hokuyo::close() {
    tcpClient.close();
}

vector<string> Hokuyo::splitStringByNewline(const string& str) {
    auto result = std::vector<std::string>{};
    auto ss = std::stringstream{str};
    
    for (std::string line; std::getline(ss, line, '\n');)
        result.push_back(line);
    
    return result;
}
