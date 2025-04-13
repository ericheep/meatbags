//
//  Hokuyo.cpp
//

#include "Hokuyo.hpp"

Hokuyo::Hokuyo() {
    reconnectionTimeout = 2.0;
    reconnectionTimer = 0.0;
    
    statusInterval = 3.0;
    statusTimer = 0.0;
    
    pollingInterval = 1.0 / 25;
    pollingTimer = 0;
    
    isConnected = false;
    laserActive = true;
    
    // 180 degrees, front facing range
    startStep = 255;
    endStep = 769;
    
    clusterCount = 0;
    
    status = "";
    lastStatus = status;
    
    // containers
    polarCoordinates.resize(1024);
    intensities.resize(1024);
    
    callIntensitiesActive = true;
    
    for (int i = 0; i < 1024; i++) {
        float theta = ((float) i / polarCoordinates.size()) * TWO_PI - HALF_PI;

        polarCoordinates[i] = ofPoint(theta, 0.0);
        intensities[i] = 0;
    }
}

void Hokuyo::setup(string _ipAddress, int _port) {
    ipAddress = _ipAddress;
    port = _port;
    
    connect();
}

void Hokuyo::connect() {
    ofxTCPSettings settings(ipAddress, port);
    tcpClient.setup(settings);
    tcpClient.setMessageDelimiter("\012\012");
}

void Hokuyo::callStatus() {
    send("II");
}

void Hokuyo::send(string msg) {
    tcpClient.send(msg + "\012");
}

void Hokuyo::activate() {
    send("BM");
}

void Hokuyo::quiet() {
    send("QT");
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

void Hokuyo::callDistances() {
    string msg = formatDistanceMessage("GD");
    send(msg);
}

void Hokuyo::callDistancesAndIntensities() {
    string msg = formatDistanceMessage("GE");
    send(msg);
}

void Hokuyo::checkStatus() {
    statusTimer += ofGetLastFrameTime();

    if (statusTimer > statusInterval){
        callStatus();
        statusTimer = 0.0;
    }
    
    if (laserActive && laserState == "OFF") {
        activate();
    }
    
    if (!laserActive && laserState == "ON") {
        quiet();
    }
}

void Hokuyo::update() {
    if(tcpClient.isConnected()) {
        if (!isConnected) {
            status = "Connected at " + tcpClient.getIP() + " " + to_string(tcpClient.getPort());
            isConnected = true;
            callStatus();
        }
        
        string response = tcpClient.receive();
        if (response.length() > 0){
            parseResponse(response);
        }
        
        checkStatus();
        
        if (laserState == "ON") {
            pollingTimer += ofGetLastFrameTime();
            if (pollingTimer > pollingInterval) {
                if (callIntensitiesActive) {
                    callDistancesAndIntensities();
                } else {
                    callDistances();
                }
                pollingTimer = 0;
            }
        }
    } else {
        if (isConnected) {
            status = "Disconnected";
            isConnected = false;
        }
        
        reconnect();
    }
    
    if (status != lastStatus) {
        cout << status << endl;
        lastStatus = status;
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
    if (command == "II") parseInfo(lines);
    if (command == "BM") parseActivate(lines);
    if (command == "QT") parseQuiet(lines);
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
        
        polarCoordinates[step].y = distance;
        
        step += 1;
    }
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
        
        polarCoordinates[step].y = distance;
        intensities[step] = intensity;

        step += 1;
    }
}

void Hokuyo::parseInfo(vector<string> packet) {
    for (int i = 1; i < packet.size(); i++) {
        string checkedLine = checkSum(packet[i], 2);
        
        if (checkedLine.size() > 5) {
            string name = checkedLine.substr(0, 4);
            string status = checkedLine.substr(5);
            
            if (name == "MODL") model = status;
            if (name == "LASR") laserState = status;
            if (name == "MESM") measurementMode = status;
            if (name == "SBPS") bitRate = status;
            if (name == "TIME") timeStamp = char2int6bitDecode(status);
            if (name == "STAT") sensorDiagnostic = status;
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
    reconnectionTimer += ofGetLastFrameTime();

    if (reconnectionTimer > reconnectionTimeout){
        connect();
        reconnectionTimer = 0;
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

void Hokuyo::getPolarCoordinates(vector<ofPoint>& _polarCoordinates) {
    for (int i = 0; i < polarCoordinates.size(); i++) {
        _polarCoordinates[i].x = polarCoordinates[i].x;
        _polarCoordinates[i].y = polarCoordinates[i].y;
    }
}

void Hokuyo::getIntensities(vector<int>& _intensities) {
    for (int i = 0; i < intensities.size(); i++) {
        _intensities[i] = intensities[i];
    }
}
