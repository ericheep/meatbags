//
//  Hokuyo.hpp
//

#ifndef Hokuyo_hpp
#define Hokuyo_hpp

#include <stdio.h>
#include "sstream"

#include "ofMain.h"
#include "ofxNetwork.h"
#include "Sensor.hpp"

class Hokuyo : public Sensor {
public:
    Hokuyo();
    ~Hokuyo();
    
    void update() override;

    void threadedFunction() override;
    void checkConnection();
    
    void sendResetStatusCommand();
    void sendSetMotorSpeedCommand(int motorSpeed);
    void sendRebootCommand();
    void sendMeasurementModeOnCommand();
    void sendMeasurementModeOffCommand();
    void sendVersionInfoCommand();
    void sendStatusInfoCommand();
    void sendParameterInfoCommand();
    void sendGetDistancesCommand();
    void sendStreamDistancesCommand();
    void sendGetDistancesAndIntensitiesCommand();
    void send(string msg);
    
    void parseResponse(const string& str);
    void parseStatusInfo(vector<string>& packet);
    void parseVersionInfo(vector<string> packet);
    void parseParameterInfo(vector<string> packet);
    void parseActivate(vector<string> packet);
    void parseQuiet(vector<string> packet);
    void parseStreamingDistances(vector<string> packet);
    void parseDistances(vector<string> packet);
    void parseDistancesAndIntensities(vector<string> packet);
    void parseMotorSpeed(vector<string> packet);
    
    string zeroPad(int value, int numberChars);
    string checkSum(string str, int fromEnd);
    
    string formatDistanceMessage(string command);
    string formatStreamDistancesMessage(string command);
    string formatIpv4String(string command);
    
    int sixBitCharDecode(const char* data, int length);
    vector<string> splitStringByNewline(const string& str);
private:
    bool callIntensitiesActive;
    int startStep, endStep, clusterCount, timeStamp;
    
    string motorSpeed;
    string measurementMode, bitRate, sensorDiagnostic;
    string vendorInfo, productInfo, firmwareVersion, protocolVersion, serialNumber;
    string minimumMeasurableDistance, maximumMeasureableDistance, angularResolutionInfo;
    string startingStep, endingStep, stepNumberOfFrontDirection, scanningSpeed;
    string scanDirection;
    
    void updateSensorInfo();
    
    string status;
    float checkTimer, checkTimeInterval;
    float streamingTimer, streamingTimeInterval;
};

#endif /* Hokuyo_hpp */
