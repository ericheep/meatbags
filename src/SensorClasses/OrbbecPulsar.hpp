//
//  OrbbecPulsar.hpp
//  meatbags

#ifndef OrbbecPulsar_hpp
#define OrbbecPulsar_hpp

#include <stdio.h>
#include "sstream"

#include "Sensor.hpp"
#include "ofxNetwork.h"

class OrbbecPulsar : public ofThread, public Sensor {
public:
    OrbbecPulsar();
    ~OrbbecPulsar();
    
    virtual void connect() override;
    virtual void reconnect() override;
    virtual void update() override;
    virtual void draw() override;
    virtual void close() override;

    void threadedFunction() override;

protected:
    void sendControlCommand(const vector<uint8_t>& command);
    void sendConnectCommand();
    void sendEnableDataStreamCommand();
    void sendDisableDataStreamCommand();
    void sendSetRotationSpeedCommand(int speed); // 15, 20, 25, 30, 40 Hz
    void sendSetTCPModeCommand();
    
    void sendGetDeviceModelCommand();
    void sendGetWorkingModeCommand();
    void sendGetWarningInformationCommand();
    void sendGetTransmissionProtocolCommand();
    void sendGetSpecialWorkingModeCommand();

    void sendGetCommand(uint16_t getCommand);
    
    bool isPointCloudData(const uint8_t* data,  int bytesRead);
    bool isControlResponse(const uint8_t* data,  int bytesRead);
    
    void parseControlResponse(const uint8_t* data,  int bytesRead);
    void parsePointCloudData(const uint8_t* data, int bytesRead);
    void parseDeviceModel(const uint8_t* data, int bytesRead);
    void parseWorkingMode(const uint8_t* data, int bytesRead);
    void extractPointCloudPoints(const uint8_t* data, int pointCount, int startAngle, float angularRes);
    
    // Utility methods
    uint8_t calculateCRC8(const vector<uint8_t>& data);
    uint16_t bytesToUint16(uint8_t low, uint8_t high);
    uint32_t bytesToUint32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
    vector<uint8_t> createControlMessage(uint16_t registerAddr, const vector<uint8_t>& dataSegment = {});
    
    ofxTCPClient tcpClient;
    
    int rotationFrequency;         // 15, 20, 25, 30, 40 Hz
    int expectedPointsPerBlock;    // Varies with rotation frequency
    int expectedBytesPerBlock;     // Varies with rotation frequency
    
    // protocol constants
    static const uint16_t GET_DEVICE_MODEL = 0x020A;
    static const uint16_t GET_FIRMWARE_VERSION = 0x020B;
    static const uint16_t GET_DEVICE_SERIAL = 0x0209;
    static const uint16_t GET_WORKING_MODE = 0x0208;
    static const uint16_t GET_TRANS_PROTOCOL = 0x0207;
    static const uint16_t GET_WARN_INFO = 0x020E;
    static const uint16_t GET_SPECIFIC_MODE = 0x0217;

    static const uint16_t REG_CONNECT_DEVICE = 0x0109;
    static const uint16_t REG_ENABLE_DATA_STREAM = 0x010F;
    static const uint16_t REG_SET_ROTATION_SPEED = 0x0105;
    static const uint16_t REG_SET_WORK_MODE = 0x0108;
    static const uint16_t REG_SET_TCP_MODE = 0x0107;
    
    // static constants
    const vector<uint8_t> FRAME_HEADER = {0x4D, 0x53, 0x02, 0xF4, 0xEB, 0x90};
    const vector<uint8_t> FRAME_FOOTER = {0xFE, 0xFE, 0xFE, 0xFE};
    
    // Orbbec-specific status
    string firmwareVersion;
    string deviceModel;
    string serialNumber;
    string workingMode;
    int currentRotationSpeed;
    float temperature;
    uint32_t timestamp;
    
    float dataStreamTimer, dataStreamTimeout;
    float reconnectionTimer, reconnectionTimeout;
    
    virtual void setIPAddress(string& ipAddress) override;
    virtual void setSleep(bool& isSleeping) override;
private:
    
};

#endif /* OrbbecPulsar_hpp */
