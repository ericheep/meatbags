//
//  Hokuyo.hpp
//

#ifndef Hokuyo_hpp
#define Hokuyo_hpp

#include <stdio.h>

#include "ofMain.h"
#include "ofxNetwork.h"
#include "sstream"

class Hokuyo : public ofThread {
public:
    Hokuyo();
    
    void draw();
    void update();
    
    void setup(string ip, int port);
    void threadedFunction() override;
    void connect();
    void reconnect();
    void close();
    
    void sendResetStatusCommand();
    void sendSetMotorSpeedCommand(int motorSpeed);
    void sendSetIPAddressCommand();
    void sendRebootCommand();
    void sendMeasurementModeOnCommand();
    void sendMeasurementModeOffCommand();
    void sendVersionInfoCommand();
    void sendStatusInfoCommand();
    void sendParameterInfoCommand();
    void sendGetDistancesCommand();
    void sendGetDistancesAndIntensitiesCommand();
    void send(string msg);

    void parseResponse(string str);
    void parseStatusInfo(vector<string> packet);
    void parseVersionInfo(vector<string> packet);
    void parseParameterInfo(vector<string> packet);
    void parseActivate(vector<string> packet);
    void parseQuiet(vector<string> packet);
    void parseDistances(vector<string> packet);
    void parseDistancesAndIntensities(vector<string> packet);
    void parseMotorSpeed(vector<string> packet);
    
    string checkSum(string str, int fromEnd);
    
    void checkStatus();
    string formatDistanceMessage(string command);
    string formatIpv4String(string command);
    
    int char2int6bitDecode(string str);
    vector<string> splitStringByNewline(const string& str);
    
    void setInfoPosition(float x, float y);

    // event functions
    void setIPAddress(string &ipAddress);
    void setPositionX(float &positionX);
    void setPositionY(float &positionY);
    void setMirrorAngles(bool &mirrorX);
    void setSensorRotation(float &sensorRotationDeg);
    
    ofParameter<string> ipAddress;
    ofParameter<float> positionX;
    ofParameter<float> positionY;
    ofParameter<bool> autoReconnectActive;
    ofParameter<bool> mirrorAngles;
    ofParameter<float> sensorRotationDeg;
    ofParameter<bool> showSensorInformation;

    void createCoordinate(int step, float distance);
    vector<float> angles;
    vector<ofPoint> coordinates;
    vector<int> intensities;
    float sensorRotationRad;
    bool newCoordinatesAvailable;
    
    ofPoint position;

private:
    ofxTCPClient tcpClient;
       
    string netmask, gateway;
    int port;
    
    bool laserActive;
    float pollingTimer, pollingInterval;
    float reconnectionTimer, reconnectionTimeout;
    float statusTimer, statusInterval;
    float lastFrameTime;
    
    bool isConnected;
    bool callIntensitiesActive;
    
    int startStep, endStep, clusterCount, angularResolution;
    
    int timeStamp;
    
    ofTrueTypeFont font;
    string model, laserState, motorSpeed;
    string measurementMode, bitRate, sensorDiagnostic;
    string connectionStatus;
    string vendorInfo, productInfo, firmwareVersion, protocolVersion, serialNumber;
    string minimumMeasurableDistance, maximumMeasureableDistance, angularResolutionInfo;
    string startingStep, endingStep, stepNumberOfFrontDirection, scanningSpeed;
    string scanDirection;
    
    string status, lastStatus;
    float x, y, width, height;
};

#endif /* Hokuyo_hpp */
