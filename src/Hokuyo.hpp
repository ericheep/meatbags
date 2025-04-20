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
    
    void update();
    void draw();
    
    void setup(string ip, int port);
    void connect();
    void reconnect();
    
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
    
    void close();
    void send(string msg);
    
    void setSensorRotation(float sensorRotation);
    void setFont(ofTrueTypeFont globalFont);
    void setRectangle(float x, float y, float width, float height);
    void setAutoReconnect(bool autoReconnectActive);
    
    void getPolarCoordinates(vector<ofPoint>& polarCoordinates);
    void getIntensities(vector<int>& intensities);
    
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
    
    int char2int6bitDecode(string str);
    
    vector<string> splitStringByNewline(const string& str);
    
    vector<ofPoint> polarCoordinates;
    vector<int> intensities;
    
    bool newCoordinatesAvailable;

private:
    ofxTCPClient tcpClient;
    
    string msgTx, msgRx;
    
    string ipAddress;
    int port;
    
    bool laserActive;
    
    float pollingTimer, pollingInterval;
    float reconnectionTimer, reconnectionTimeout;
    float statusTimer, statusInterval;
    float lastFrameTime;
    float sensorRotation;
    
    bool isConnected, autoReconnectActive;
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
