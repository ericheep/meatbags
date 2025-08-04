//
//  Sensor.hpp
//  meatbags

#ifndef Sensor_hpp
#define Sensor_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxDropdown.h"

#include "DraggablePoint.hpp"

class Sensor {
public:
    Sensor();
    ~Sensor();
    
    // virtual functions
    virtual void connect() = 0;
    virtual void reconnect() = 0;
    virtual void update() = 0;
    virtual void close() = 0;
    virtual void draw() = 0;
    
    // set drawing rectangle
    void setInfoPosition(float x, float y);
    
    // connection event functions
    virtual void setIPAddress(string& ipAddress);

    // event functions
    void setInterfaceAndIP(string interface, string localIP);
    void setLocalIPAddress(string& ipLocalAddress);
    void setPositionX(float& positionX);
    void setPositionY(float& positionY);
    void setMirrorAngles(bool& mirrorX);
    void setSensorRotation(float& sensorRotationDeg);
    
    // interactions
    DraggablePoint position, nosePosition;
    float mouseBoxSize, mouseBoxHalfSize, noseRadius;
    float mouseNoseBoxSize, mouseNoseBoxHalfSize, mouseNoseBoxRadius;
    bool isMouseOver, isMouseClicked, isMouseOverNose, isMouseOverNoseClicked;
    bool threadActive;
    string model;
    string logStatus, logConnectionStatus, logMode;
    
    // parameters
    ofxDropdown_<string> typesDropdown { "types" };
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
    
    string laserState;
    
    float lastFrameTime;
    
    int angularResolution;
    
    // for the viewer class
    bool isConnected;
protected:
    virtual void initializeVectors();
    virtual void setupParameters();
};

#endif /* Sensor_hpp */
