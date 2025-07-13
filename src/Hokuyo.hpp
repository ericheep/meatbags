//
//  Hokuyo.hpp
//

#ifndef Hokuyo_hpp
#define Hokuyo_hpp

#include <stdio.h>

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxNetwork.h"
#include "DraggablePoint.hpp"
#include "MemoryFont.hpp"
#include "sstream"

class Hokuyo : public ofThread {
public:
    Hokuyo();
    ~Hokuyo();
    
    void draw();
    void update();
    
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
    
    void checkStatus();
    string formatDistanceMessage(string command);
    string formatStreamDistancesMessage(string command);
    string formatIpv4String(string command);
    
    int sixBitCharDecode(const char* data, int length);
    vector<string> splitStringByNewline(const string& str);
    
    void setInfoPosition(float x, float y);

    // event functions
    void setInterfaceAndIP(string interface, string localIP);
    void setLocalIPAddress(string& localIPAddress);
    void setIPAddress(string& ipAddress);
    void setPositionX(float& positionX);
    void setPositionY(float& positionY);
    void setMirrorAngles(bool& mirrorX);
    void setSensorRotation(float& sensorRotationDeg);
    void setSleep(bool& isSleeping);

    ofParameter<string> ipAddress;
    ofParameter<float> positionX;
    ofParameter<float> positionY;
    ofParameter<bool> autoReconnectActive;
    ofParameter<bool> mirrorAngles;
    ofParameter<bool> isSleeping;

    ofParameter<float> sensorRotationDeg;
    ofParameter<bool> showSensorInformation;
    ofParameter<ofColor> sensorColor;
    ofParameter<int> whichMeatbag;
    
    void createCoordinate(int step, float distance);
    vector<float> angles;
    vector<ofPoint> coordinates;
    vector<int> intensities;
    float sensorRotationRad;
    bool newCoordinatesAvailable;
    
    DraggablePoint position, nosePosition;
    float mouseBoxSize, mouseBoxHalfSize, noseRadius;
    float mouseNoseBoxSize, mouseNoseBoxHalfSize, mouseNoseBoxRadius;
    bool isMouseOver, isMouseClicked, isMouseOverNose, isMouseOverNoseClicked;
    bool isConnected, threadActive;
    
    int index;
    float streamingPollingTimer, streamingPollingInterval;
    string model, status, connectionStatus, laserState;
private:
    ofxTCPClient tcpClient;
       
    string netmask, gateway, localIPAddress, interface;
    int port;
    
    float reconnectionTimer, reconnectionTimeout;
    float statusTimer, statusInterval;
    float threadInactiveTime, threadInactiveInterval;
    float lastFrameTime;
    
    bool callIntensitiesActive;
    int startStep, endStep, clusterCount, angularResolution;
    int timeStamp;
    
    MemoryFont font;
    string motorSpeed, lastStatus;
    string measurementMode, bitRate, sensorDiagnostic;
    string vendorInfo, productInfo, firmwareVersion, protocolVersion, serialNumber;
    string minimumMeasurableDistance, maximumMeasureableDistance, angularResolutionInfo;
    string startingStep, endingStep, stepNumberOfFrontDirection, scanningSpeed;
    string scanDirection;
    
    float x, y, width, height;
};

#endif /* Hokuyo_hpp */
