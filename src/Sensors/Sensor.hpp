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
    virtual void connect() = 0;
    virtual void reconnect() = 0;
    virtual void update() = 0;
    virtual void close() = 0;
    virtual void draw() = 0;
    virtual void shutdown() = 0;
    
    // set drawing rectangle
    void setInfoPosition(float x, float y);
    void drawSensorInfo(vector<string> sensorInfoLines);
    
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
    float threadInactiveTimer, threadInactiveTimeInterval;
    float reconnectionTimer, reconnectionTimeInterval;
    
    string model;
    string logStatus, logConnectionStatus, logMode;
    bool autoReconnectActive;
    ofColor sensorColor;

    // parameters
    ofxDropdown_<string> sensorTypes { "types" };
    ofParameter<string> ipAddress;
    ofParameter<float> positionX;
    ofParameter<float> positionY;
    ofParameter<bool> mirrorAngles;
    ofParameter<bool> isSleeping;
    ofParameter<float> sensorRotationDeg;
    ofParameter<bool> showSensorInformation;
    ofParameter<int> whichMeatbag;
    ofParameter<int> guiMotorSpeed;
    ofParameter<bool> guiSpecialWorkingMode;
    
    void createCoordinate(int step, float distance);
    vector<float> angles;
    vector<ofPoint> coordinates;
    vector<int> intensities;
    float sensorRotationRad;
    bool newCoordinatesAvailable;
    
    float x, y, width, height;
    string netmask, gateway, localIPAddress, interface;
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

    virtual void initializeVectors();
    virtual void setupParameters();
    
    ofxTCPClient tcpClient;
};

#endif /* Sensor_hpp */
