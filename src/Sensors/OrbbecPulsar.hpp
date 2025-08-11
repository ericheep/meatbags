//
//  OrbbecPulsar.hpp
//  meatbags

#ifndef OrbbecPulsar_hpp
#define OrbbecPulsar_hpp

#include <stdio.h>
#include "sstream"
#include "ofxNetwork.h"
#include "Sensor.hpp"

class OrbbecPulsar : public Sensor {
public:
    OrbbecPulsar();
    ~OrbbecPulsar();
    
    virtual void update() override;

    void threadedFunction() override;

private:
    uint8_t receiveBuffer[1024];
    mutable string lineBuffer;
protected:
    void sendControlCommand(const vector<uint8_t>& command);
    
    void sendConnectCommand();
    void sendEnableDataStreamCommand();
    void sendDisableDataStreamCommand();
    void sendSetStandbyMode();
    void sendSetRangingMode();
    void sendSetMotorSpeedCommand(int speed);
    void sendSetTCPModeCommand();
    void sendSetFogModeCommand();
    void sendSetNormalModeCommand();

    void sendGetMotorSpeedCommand();
    void sendGetTransmissionProtocolCommand();
    void sendGetWorkingModeCommand();
    void sendGetSerialNumberCommand();
    void sendGetDeviceModelCommand();
    void sendGetFirmwareVersionCommand();
    void sendGetLidarWarningCommand();
    void sendGetSpecialWorkingModeCommand();

    void sendNextStatusCommand();
    void sendGetCommand(uint16_t getCommand);
    vector<std::function<void()>> statusCommands;
    int currentStatusCommandIndex;
    
    bool isPointCloudData(const uint8_t* data,  int bytesRead);
    bool isControlResponse(const uint8_t* data,  int bytesRead);
    void extractPointCloudPoints(const uint8_t* data, int pointCount, int startAngle, float angularRes);

    void parseMotorSpeed(const uint8_t* data,  int bytesRead);
    void parseControlResponse(const uint8_t* data,  int bytesRead);
    void parseTransmissionProtocol(const uint8_t* data, int bytesRead);
    void parsePointCloudData(const uint8_t* data, int bytesRead);
    void parseDeviceModel(const uint8_t* data, int bytesRead);
    void parseWorkingMode(const uint8_t* data, int bytesRead);
    void parseSerialNumber(const uint8_t* data, int bytesRead);
    void parseFirmwareVersion(const uint8_t* data, int bytesRead);
    void parseLidarWarning(const uint8_t* data, int bytesRead);
    void parseSpecialWorkingMode(const uint8_t* data, int bytesRead);
    void parseString(const uint8_t* data,  int bytesRead, string& value);

    void checkMotorSpeed();
    void checkTransmissionProtocol();
    void checkWorkingMode();
    void checkSpecialWorkingMode();
    
    void checkNextSetting();
    vector<std::function<void()>> checkCommands;
    int currentCheckCommandIndex;
    
    // Utility methods
    uint8_t calculateCRC8(const vector<uint8_t>& data);
    uint8_t calculateCRC8Fast(const uint8_t* data, size_t length);
    uint16_t bytesToUint16(uint8_t low, uint8_t high);
    uint32_t bytesToUint32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
    void sendControlMessage(uint16_t registerAddr, std::initializer_list<uint8_t> data);
        
    int motorSpeed;
    int rotationFrequency;         // 15, 20, 25, 30, 40 Hz
    int expectedPointsPerBlock;    // Varies with rotation frequency
    int expectedBytesPerBlock;     // Varies with rotation frequency
    
    // protocol constants
    static const uint16_t GET_MOTOR_SPEED = 0x0205;
    static const uint16_t GET_TRANS_PROTOCOL = 0x0207;
    static const uint16_t GET_WORKING_MODE = 0x0208;
    static const uint16_t GET_DEVICE_SERIAL = 0x0209;
    static const uint16_t GET_DEVICE_MODEL = 0x020A;
    static const uint16_t GET_FIRMWARE_VERSION = 0x020B;
    static const uint16_t GET_LIDAR_WARNING = 0x020E;
    static const uint16_t GET_SPECIFIC_MODE = 0x0217;
    
    static const uint16_t REG_CONNECT_DEVICE = 0x0109;
    static const uint16_t REG_SET_DATA_STREAM = 0x010F;
    static const uint16_t REG_SET_MOTOR_SPEED = 0x0105;
    static const uint16_t REG_SET_WORKING_MODE = 0x0108;
    static const uint16_t REG_SET_TCP_MODE = 0x0107;
    static const uint16_t REG_SET_SPECIAL_WORKING_MODE = 0x0110;

    // static constants
    const vector<uint8_t> FRAME_HEADER = {0x4D, 0x53, 0x02, 0xF4, 0xEB, 0x90};
    const vector<uint8_t> FRAME_FOOTER = {0xFE, 0xFE, 0xFE, 0xFE};
    
    // Orbbec-specific status
    string firmwareVersion;
    string serialNumber;
    string workingMode;
    string lidarWarning;
    string specialWorkingMode;
    string transmissionProtocol;
    int currentRotationSpeed;
    float temperature;
    uint32_t timestamp;
    string lidarState;
    
    void updateSensorInfo();
   
    vector<uint8_t> commandBuffer;

    float checkTimer, checkTimeInterval;
    std::mutex sensorDataMutex;
};

#endif /* OrbbecPulsar_hpp */
