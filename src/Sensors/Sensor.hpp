//
//  Sensor.hpp
//  meatbags

#ifndef Sensor_hpp
#define Sensor_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxDropdown.h"
#include "ofxNetwork.h"
#include "Space.h"

#include "DraggablePoint.hpp"

class Sensor : public ofThread {
public:
    Sensor();
    virtual ~Sensor();
    
    // virtual functions
    virtual void update() = 0;
    
    void draw();
    void connect();
    void checkIfReconnect();
    void checkIfThreadRunning();
    bool tcpSetup();
    
    // set drawing rectangle
    void setInfoPosition(float x, float y);
    void drawSensorInfo();
    
    // connection event functions
    virtual void setIPAddress(string& ipAddress);

    // event functions
    void setInterfaceAndIP(string interface, string localIP);
    void setLocalIPAddress(string& ipLocalAddress);
    void setPositionX(float& positionX);
    void setPositionY(float& positionY);
    void setMirrorAngles(bool& mirrorX);
    void setSensorRotation(float& sensorRotationDeg);
    void setSpace(Space& space);
    void setTranslation(ofPoint translation);

    ofPoint convertCoordinateToScreenPoint(ofPoint coordinate);
    ofPoint convertScreenPointToCoordinate(ofPoint screenPoint);
    
    bool onMouseMoved(ofMouseEventArgs & mouseArgs);
    bool onMousePressed(ofMouseEventArgs & mouseArgs);
    bool onMouseDragged(ofMouseEventArgs & mouseArgs);
    bool onMouseReleased(ofMouseEventArgs & mouseArgs);
    bool onKeyPressed(ofKeyEventArgs & keyArgs);
    
    // interactions
    DraggablePoint position, nosePosition;
    float mouseBoxSize, mouseBoxHalfSize, noseRadius;
    float mouseNoseBoxSize, mouseNoseBoxHalfSize, mouseNoseBoxRadius;
    bool isMouseOver, isMouseClicked, isMouseOverNose, isMouseOverNoseClicked;
    bool threadActive;
    
    float statusTimer, statusTimeInterval;
    atomic<float> threadInactiveTimer{0.0};
    float threadInactiveTimeInterval;
    float reconnectionTimer, reconnectionTimeInterval;
    
    string model;
    string logStatus, logConnectionStatus, logMode, connectionStatus;
    bool autoReconnectActive;
    ofColor sensorColor;
    
    // in update loop
    ofxDropdown_<string> sensorTypes { "types" };
    ofParameter<string> ipAddress;
    ofParameter<float> positionX;
    ofParameter<float> positionY;
    ofParameter<bool> mirrorAngles;
    ofParameter<bool> isSleeping;
    ofParameter<float> sensorRotationDeg;
    ofParameter<bool> showSensorInformation;
    ofParameter<int> whichMeatbag;

    // in threads
    ofParameter<int> guiMotorSpeed;
    ofParameter<bool> guiSpecialWorkingMode;
    vector<string> sensorInfoLines;
    
    void createCoordinate(int step, float distance);
    vector<float> angles;
    
    vector<int> intensities;
    float sensorRotationRad;
    
    float x, y, width, height;
    string localIPAddress, interface;
    int port;
    int index;
    
    float scale;
    ofPoint translation;
    Space space;
    string laserState;
    
    float lastFrameTime;
    int angularResolution;
    
    // for the viewer class
    bool isConnected;
    bool isShuttingDown;
    
    virtual void initializeVectors();
    virtual void setupParameters();
    
    ofxTCPClient tcpClient;
    
    void updateDistances();
    vector<ofPoint> coordinates;
protected:
    vector<float> distances;
    vector<float> cachedDistances;
    
    mutable std::mutex distancesMutex;
};

#endif /* Sensor_hpp */
