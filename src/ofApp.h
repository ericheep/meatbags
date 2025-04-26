#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxOsc.h"

#include "Hokuyo.hpp"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "Viewer.hpp"
#include "Space.h"
#include "Sensors.hpp"

class ofApp : public ofBaseApp{
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void windowResized(int width, int height) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;

    void drawFps();
    
    // hokuyo parameters
    void setPositionX(float &x);
    void setPositionY(float &y);
    void setAutoReconnect(bool &autoReconnectActive);
    void setSensorRotation(float &sensorRotation);
    void setMirrorAngles(bool &mirrorAngles);
    
    // bounds parameters
    void updateGuiBounds();
    void setSpace();
    void setBoundsX1(float &boundsX1);
    void setBoundsX2(float &boundsX2);
    void setBoundsY1(float &boundsY1);
    void setBoundsY2(float &boundsY2);
    
    // sensors
    void addSensor();
    void removeSensor();
    void setNumberSensors(int & numberSensors);
    
    // view parameters
    void setAreaSize(float &areaSize);
    
    // osc parameters
    void setOscSenderAddress(string &oscSenderAddress);
    void setOscSenderPort(int &oscSenderAddress);
    
    void sendBlobOsc();
    
    Sensors sensors;
    vector<Blob> blobs;
    Meatbags meatbags;
    Space space;
    Bounds bounds;
    Viewer viewer;
    ofxOscSender oscSender;
    
    ofxPanel gui;
    ofPoint origin;
    
    // meatbags parameters
    ofParameterGroup meatbagsSettings;
    ofParameter<float> epsilon;
    ofParameter<int> minPoints;
    ofParameter<float> blobPersistence;
    
    // viewer parameters
    ofParameter<float> areaSize;
    
    // bounds parameters
    ofParameter<float> boundsX1;
    ofParameter<float> boundsX2;
    ofParameter<float> boundsY1;
    ofParameter<float> boundsY2;
    
    // osc parameters
    ofParameterGroup oscSettings;
    ofParameter<string> oscSenderAddress;
    ofParameter<int> oscSenderPort;
    ofParameter<bool> normalizeBlobs;
    ofParameter<bool> oscActive;
    
    ofParameter<int> numberSensors;

    ofParameterGroup sensorsSettings;
    vector<ofParameterGroup> sensorSettings;
    vector<ofColor> sensorColors;
};
