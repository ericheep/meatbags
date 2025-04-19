#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxOsc.h"

#include "Hokuyo.hpp"
#include "Blob.hpp"
#include "Meatbags.hpp"

class ofApp : public ofBaseApp{
    
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void windowResized(int width, int height) override;

    void drawFps();
    
    void setAutoReconnect(bool &autoReconnectActive);
    void setAreaSize(float &areaSize);
    void setBlobPersistence(float &blobPersistence);
    void setEpsilon(float &epsilon);
    void setMinPoints(int &minPoints);
    void setOscSenderAddress(string &oscSenderAddress);
    void setOscSenderPort(int &oscSenderAddress);
    
    void sendBlobOsc();

    Hokuyo hokuyo;
    
    // blob tracking software
    Meatbags meatbags;
    vector <Blob> blobs;
    ofTrueTypeFont globalFont;
    
    ofxPanel gui;
    ofParameter<float> areaSize;
    ofParameter<float> epsilon;
    ofParameter<int> minPoints;
    ofParameter<float> blobPersistence;
    
    ofParameter<string> oscSenderAddress;
    ofParameter<int> oscSenderPort;
    ofParameter<bool> normalizeBlobs;
    ofParameter<bool> oscActive;
    ofParameter<bool> autoReconnectActive;
    ofParameter<bool> showSensorInformation;

    ofxOscSender oscSender;
};
