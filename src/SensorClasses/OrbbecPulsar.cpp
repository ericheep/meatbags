//
//  OrbbecPulsar.cpp
//  meatbags

#include "OrbbecPulsar.hpp"

OrbbecPulsar::OrbbecPulsar() : Sensor() {
    angularResolution = int(360.0 / 0.1);      // default for 20hz mode
    port = 2228;                   // Orbbec default UDP port
    rotationFrequency = 20;        // default to 20hz
    
    currentRotationSpeed = 0;
    temperature = 0.0;
    timestamp = 0;
    
    initializeVectors();
    setupParameters();
    
    dataStreamTimer = 0.0;
    dataStreamTimeout = 1.0;
    
    threadInactiveInterval = 5.0;
    
    reconnectionTimeout = 5.0;
    reconnectionTimer = 0.0;
    
    statusTimer = 0.0;
    statusInterval = 3.0;
    
    ipAddress = "192.168.1.100";
    
    stopThread();
    sleep(1000);
}

OrbbecPulsar::~OrbbecPulsar() {
    close();
}

void OrbbecPulsar::reconnect() {
    connect();
}

void OrbbecPulsar::connect() {
    if (ipAddress.get() == "0.0.0.0" || ipAddress.get().empty()) {
        connectionStatus = "No IP address specified";
        return;
    }
    
    connectionStatus = "Connecting to Orbbec at " + ipAddress.get();
    
    if (!isThreadRunning()) {
        startThread();
    } else {
        ofLogWarning("OrbbecPulsar") << "Thread already running; ignoring connect() call.";
    }
}

void OrbbecPulsar::threadedFunction() {
    bool tcpConnected = tcpClient.setup(ipAddress.get(), port);
    
    if (!tcpConnected) {
        connectionStatus = "Failed to connect TCP control";
        return;
    }
    
    sendConnectCommand();
    
    while (isThreadRunning() && tcpClient.isConnected()) {
        uint8_t data[1024];
        int bytesRead = tcpClient.receiveRawBytes((char*)data, sizeof(data));
         
        if (bytesRead > 0) {
            // cout << "BYTES READ " << bytesRead << endl;
            if (isControlResponse(data, bytesRead)) {
                parseControlResponse(data, bytesRead);
            } else if (isPointCloudData(data, bytesRead)){
                parsePointCloudData(data, bytesRead);
            }
        }
        
        threadInactiveTime = 0;
        sleep(1);
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
    statusTimer += lastFrameTime;
    
    // Check thread activity
    threadInactiveTime += lastFrameTime ;
    if (threadInactiveTime > threadInactiveInterval ) {
        if (isThreadRunning()) {
            stopThread();
        }
        cout << threadInactiveTime << endl;
        startThread();
        threadInactiveTime = 0;
    }
    
    if (statusTimer > statusInterval) {
        sendGetDeviceModelCommand();
        sendGetWorkingModeCommand();
        statusTimer = 0;
    }
    
    isConnected = tcpClient.isConnected();
    if (!tcpClient.isConnected()) {
        reconnectionTimer += lastFrameTime;
        
        connectionStatus = "Disconnected";
        status = "Orbbec sensor disconnected";
        if (reconnectionTimer > reconnectionTimeout) {
            reconnect();
            reconnectionTimer = 0;
            ofLogNotice() << "Attemping reconnect";
        }
    }
}

void OrbbecPulsar::draw() {
    // Sensor::draw();
    
    if (!showSensorInformation) return;
    
    // Add Orbbec-specific information display
    ofSetColor(ofColor::grey);
    
    vector<string> sensorInfoLines;
    sensorInfoLines.push_back("Model: " + deviceModel);
    sensorInfoLines.push_back("Firmware: " + firmwareVersion);
    sensorInfoLines.push_back("Serial: " + serialNumber);
    sensorInfoLines.push_back("Rotation Speed: " + to_string(currentRotationSpeed) + " RPM");
    sensorInfoLines.push_back("Temperature: " + to_string(temperature) + "°C");
    sensorInfoLines.push_back("IP Address: " + ipAddress.get());
    sensorInfoLines.push_back("Port: " + to_string(port));
    sensorInfoLines.push_back("Connection: " + connectionStatus);
    sensorInfoLines.push_back("Status: " + status);
    sensorInfoLines.push_back("Timestamp: " + to_string(timestamp));
    
    // Draw info box (similar to Hokuyo implementation)
    float textBoxHeight = sensorInfoLines.size() * 16;
    float textBoxWidth = 300;
    
    float textX = x - textBoxWidth * 0.5;
    float textY = y - textBoxHeight * 0.5;
    
    ofFill();
    ofSetColor(0, 0, 0, 200);
    ofRectangle textBox;
    textBox.setFromCenter(x, y, textBoxWidth, textBoxHeight + 8);
    ofDrawRectangle(textBox);
    ofSetColor(ofColor::lightBlue);
    ofNoFill();
    ofDrawRectangle(textBox);
    ofFill();
    
    for (int i = 0; i < sensorInfoLines.size(); i++) {
        float yPos = textY + 16 * i;
        // You might need to adjust this based on your font setup
        ofDrawBitmapString(sensorInfoLines[i], textX + 6, yPos + 10);
    }
}

void OrbbecPulsar::close() {
    if (isThreadRunning()) {
        sendDisableDataStreamCommand();
        sleep(100);
        stopThread();
    }
    
    tcpClient.close();
}

void OrbbecPulsar::setIPAddress(string &ipAddress) {
    tcpClient.close();
    if (isThreadRunning()) {
        stopThread();
    }
    connect();
}

void OrbbecPulsar::setSleep(bool& isSleeping) {
    if (isSleeping) {
       // sendMeasurementModeOffCommand();
    } else {
       // sendMeasurementModeOnCommand();
       // sendStreamDistancesCommand();
    }
}

void OrbbecPulsar::sendGetDeviceModelCommand() {
    sendGetCommand(GET_DEVICE_MODEL);
}

void OrbbecPulsar::sendGetWorkingModeCommand() {
    sendGetCommand(GET_WORKING_MODE);
}

void OrbbecPulsar::sendGetWarningInformationCommand() {
    sendGetCommand(GET_WARN_INFO);
}

void OrbbecPulsar::sendGetTransmissionProtocolCommand() {
    sendGetCommand(GET_TRANS_PROTOCOL);
}

void OrbbecPulsar::sendGetSpecialWorkingModeCommand() {
    sendGetCommand(GET_SPECIFIC_MODE);
}

void OrbbecPulsar::sendGetCommand(uint16_t getCommand) {
    vector<uint8_t> command = {
        0x01, 0xFE,                             // frame header
        0x01,                                   // protocol version
        0x00, 0x00,                             // data segment length
        (uint8_t)((getCommand >> 8) & 0xFF),    // data length high byte
        (uint8_t)(getCommand & 0xFF),           // data length low byte
        0x00, 0x00,                             // response code
    };
    
    uint8_t crc = calculateCRC8(command);
    command.push_back(crc);
    
    sendControlCommand(command);
}

// Control command methods
void OrbbecPulsar::sendConnectCommand() {
    // Connect Device: Register 0x0109 with data 0x12345678
    vector<uint8_t> data = {0x12, 0x34, 0x56, 0x78}; // big endian
    vector<uint8_t> command = createControlMessage(REG_CONNECT_DEVICE, data);
    sendControlCommand(command);
}

void OrbbecPulsar::sendEnableDataStreamCommand() {
    // Enable Data Stream: Register 0x010F with data 0x00000001
    vector<uint8_t> data = {0x00, 0x00, 0x00, 0x01}; // big endian
    vector<uint8_t> command = createControlMessage(REG_ENABLE_DATA_STREAM, data);
    sendControlCommand(command);
}

void OrbbecPulsar::sendDisableDataStreamCommand() {
    // Disable Data Stream: Register 0x010F with data 0x00000000
    vector<uint8_t> data = {0x00, 0x00, 0x00, 0x00}; // big endian
    vector<uint8_t> command = createControlMessage(REG_ENABLE_DATA_STREAM, data);
    sendControlCommand(command);
}

void OrbbecPulsar::sendSetRotationSpeedCommand(int speed) {
    // map frequency to register values based on documentation
    uint32_t speedValue;
    switch(speed) {
        case 15: speedValue = 900; break;   // 900 RPM for 15Hz
        case 20: speedValue = 1200; break;  // 1200 RPM for 20Hz
        case 25: speedValue = 1500; break;  // 1500 RPM for 25Hz
        case 30: speedValue = 1800; break;  // 1800 RPM for 30Hz
        case 40: speedValue = 2400; break;  // 2400 RPM for 40Hz
        default: speedValue = 900; break;   // Default to 15Hz
    }
    
    vector<uint8_t> data = {
        (uint8_t)((speedValue >> 24) & 0xFF),
        (uint8_t)((speedValue >> 16) & 0xFF),
        (uint8_t)((speedValue >> 8) & 0xFF),
        (uint8_t)(speedValue & 0xFF)
    };
    
    vector<uint8_t> command = createControlMessage(REG_SET_ROTATION_SPEED, data);
    sendControlCommand(command);
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

vector<uint8_t> OrbbecPulsar::createControlMessage(uint16_t registerAddr, const vector<uint8_t>& dataSegment) {
    uint16_t dataLength = dataSegment.size();
        
    vector<uint8_t> message = {
            0x01, 0xFE,                            // frame header
            0x01,                                  // protocol version
            (uint8_t)((dataLength >> 8) & 0xFF),   // data length high byte
            (uint8_t)(dataLength & 0xFF),          // data length low byte
            (uint8_t)((registerAddr >> 8) & 0xFF), // register high byte
            (uint8_t)(registerAddr & 0xFF),        // register low byte
            0x00, 0x00                             // response code
        };
        
    // append data segment
    message.insert(message.end(), dataSegment.begin(), dataSegment.end());
        
    uint8_t crc = calculateCRC8(message);
    message.push_back(crc);
        
    return message;
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
                cout << "PARSE CONNECT DEVICE" << endl;
                sendSetTCPModeCommand();
                break;
            }
            case REG_SET_TCP_MODE: {
                cout << "PARSE TCP MODE" << endl;
                sendEnableDataStreamCommand();
                sendSetRotationSpeedCommand(15);
                break;
            }
            case REG_ENABLE_DATA_STREAM: {
                cout << "PARSE ENABLE DATA STREAM" << endl;
                break;
            }
            case REG_SET_ROTATION_SPEED: {
                sendEnableDataStreamCommand();
                cout << "PARSE SET ROTATION STREAM" << endl;
                break;
            }
            case GET_DEVICE_MODEL: {
                cout << "PARSE GET DEVICE MODEL" << endl;
                parseDeviceModel(data, bytesRead);
                break;
            }
            case GET_WORKING_MODE: {
                cout << "PARSE GET WORKING MODE" << endl;
                // parseWorkingMode(data, bytesRead);
                break;
            }
            case REG_SET_WORK_MODE: break;
            default:  break;
        }
    }
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
        
    string modeStr = "";
    switch (modeValue) {
        case 0:
            modeStr = "Ranging Mode";
            workingMode = "ranging";
            break;
        case 1:
            modeStr = "Standby Mode";
            workingMode = "standby";
            break;
        default:
            modeStr = "Unknown Mode (" + to_string(modeValue) + ")";
            workingMode = "unknown";
            break;
    }
    
    cout << workingMode << endl;
}

void OrbbecPulsar::parseDeviceModel(const uint8_t* data, int bytesRead) {
    uint16_t dataLength = bytesToUint16(data[3], data[4]);
    
    if (dataLength == 0 || bytesRead < (9 + dataLength)) {
        ofLogWarning("OrbbecPulsar") << "Invalid device model response length";
        return;
    }
    
    string modelStr = "";
    for (int i = 9; i < 9 + dataLength && i < bytesRead; i++) {
        if (data[i] != 0) {
            modelStr += char(data[i]);
        }
    }
    
    modelStr.erase(std::find_if(modelStr.rbegin(), modelStr.rend(),
                   [](unsigned char ch) { return !std::isspace(ch); }).base(),
                   modelStr.end());
    
    //replace(modelStr.begin(), modelStr.end(), '\\', ' ');
    model = modelStr;
}

void OrbbecPulsar::parsePointCloudData(const uint8_t* data, int length) {
    // extract frame length (bytes 6-7)
    uint16_t frameLength = bytesToUint16(data[6], data[7]);
    if (length < frameLength) return; // incomplete frame
    
    // Extract angle information (bytes 8-13)
    uint16_t startAngle = bytesToUint16(data[8], data[9]);    // Unit: 0.01°
    uint16_t endAngle = bytesToUint16(data[10], data[11]);    // Unit: 0.01°
    uint16_t angularRes = bytesToUint16(data[12], data[13]);  // Unit: 0.001°
    
    uint8_t infoType = data[14];      // 1=15Hz, 2=20Hz, 3=25Hz, 4=30Hz, 5=40Hz
    uint8_t blockNumber = data[15];   // 1-18
    uint16_t blockSequence = bytesToUint16(data[16], data[17]);
    
    // extract timestamp (bytes 18-21)
    timestamp = bytesToUint32(data[18], data[19], data[20], data[21]);
    
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
    newCoordinatesAvailable = true;
    threadInactiveTime = 0;
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
            
            bool m = mirrorAngles;
            setMirrorAngles(m);
        }
        
        // convert to coordinate index (map 270° range to our angular resolution)
        int coordIndex = (int)((pointAngle - 45.0f) / 360.0f * angularResolution);
        
        if (coordIndex >= 0 && coordIndex < angularResolution) {
            createCoordinate(coordIndex, distance);
            
            if (coordIndex < intensities.size()) {
                intensities[coordIndex] = intensity;
            }
        }
    }
}

uint8_t OrbbecPulsar::calculateCRC8(const vector<uint8_t>& data) {
    // CRC8 implementation from Orbbec documentation
    static const uint8_t CrcTable[256] = {
        0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3, 0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11, 0x5c, 0xa9, 0xe4, 0x33, 0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8, 0xf5, 0x1f, 0x52, 0x85, 0xc8, 0x66, 0x2b, 0xfc, 0xb1, 0xed, 0xa0, 0x77, 0x3a, 0x94, 0xd9, 0x0e, 0x43, 0xb6, 0xfb, 0x2c, 0x61, 0xcf, 0x82, 0x55, 0x18, 0x44, 0x09, 0xde, 0x93, 0x3d, 0x70, 0xa7, 0xea, 0x3e, 0x73, 0xa4, 0xe9, 0x47, 0x0a, 0xdd, 0x90, 0xcc, 0x81, 0x56, 0x1b, 0xb5, 0xf8, 0x2f, 0x62, 0x97, 0xda, 0x0d, 0x40, 0xee, 0xa3, 0x74, 0x39, 0x65, 0x28, 0xff, 0xb2, 0x1c, 0x51, 0x86, 0xcb, 0x21, 0x6c, 0xbb, 0xf6, 0x58, 0x15, 0xc2, 0x8f, 0xd3, 0x9e, 0x49, 0x04, 0xaa, 0xe7, 0x30, 0x7d, 0x88, 0xc5, 0x12, 0x5f, 0xf1, 0xbc, 0x6b, 0x26, 0x7a, 0x37, 0xe0, 0xad, 0x03, 0x4e, 0x99, 0xd4, 0x7c, 0x31, 0xe6, 0xab, 0x05, 0x48, 0x9f, 0xd2, 0x8e, 0xc3, 0x14, 0x59, 0xf7, 0xba, 0x6d, 0x20, 0xd5, 0x98, 0x4f, 0x02, 0xac, 0xe1, 0x36, 0x7b, 0x27, 0x6a, 0xbd, 0xf0, 0x5e, 0x13, 0xc4, 0x89, 0x63, 0x2e, 0xf9, 0xb4, 0x1a, 0x57, 0x80, 0xcd, 0x91, 0xdc, 0x0b, 0x46, 0xe8, 0xa5, 0x72, 0x3f, 0xca, 0x87, 0x50, 0x1d, 0xb3, 0xfe, 0x29, 0x64, 0x38, 0x75, 0xa2, 0xef, 0x41, 0x0c, 0xdb, 0x96, 0x42, 0x0f, 0xd8, 0x95, 0x3b, 0x76, 0xa1, 0xec, 0xb0, 0xfd, 0x2a, 0x67, 0xc9, 0x84, 0x53, 0x1e, 0xeb, 0xa6, 0x71, 0x3c, 0x92, 0xdf, 0x08, 0x45, 0x19, 0x54, 0x83, 0xce, 0x60, 0x2d, 0xfa, 0xb7, 0x5d, 0x10, 0xc7, 0x8a, 0x24, 0x69, 0xbe, 0xf3, 0xaf, 0xe2, 0x35, 0x78, 0xd6, 0x9b, 0x4c, 0x01, 0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17, 0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8
    };
    
    uint8_t crc = 0;
    for (uint8_t byte : data) {
        crc = CrcTable[(crc ^ byte) & 0xff];
    }
    return crc;
}

uint16_t OrbbecPulsar::bytesToUint16(uint8_t high, uint8_t low) {
    return (high << 8) | low;
}

uint32_t OrbbecPulsar::bytesToUint32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}
