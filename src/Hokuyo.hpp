//
//  Hokuyo.hpp
//

#ifndef Hokuyo_hpp
#define Hokuyo_hpp

#include <stdio.h>

#include "ofMain.h"
#include "ofxNetwork.h"
#include "sstream"

class Hokuyo {
public:
    Hokuyo();
    void setup(string ip, int port);
    void update();
    void connect();
    void reconnect();
    void activate();
    void quiet();
    void callDistances();
    void callDistancesAndIntensities();
    void close();
    void send(string msg);
    
    void getPolarCoordinates(vector<ofPoint>& polarCoordinates);
    void getIntensities(vector<int>& intensities);
    
    void parseResponse(string str);
    void parseInfo(vector<string> packet);
    void parseActivate(vector<string> packet);
    void parseQuiet(vector<string> packet);
    void parseDistances(vector<string> packet);
    void parseDistancesAndIntensities(vector<string> packet);
    
    string checkSum(string str, int fromEnd);
    
    void checkStatus();
    void callStatus();
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
    
    bool isConnected;
    bool callIntensitiesActive;
    
    int startStep, endStep, clusterCount;
    
    int timeStamp;
    string model, laserState, motorSpeed;
    string measurementMode, bitRate, sensorDiagnostic;
    
    string status, lastStatus;
};

#endif /* Hokuyo_hpp */
