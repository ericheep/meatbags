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
    void drawFps();
    
    void setOscSenderAddress(string &oscSenderAddress);
    void setOscSenderPort(int &oscSenderAddress);
    
    void sendBlobOsc();

    Hokuyo hokuyo;
    
    // blob tracking software
    Meatbags meatbags;
    vector <Blob> blobs;
    
    ofxPanel gui;
    ofParameter<float> scale;
    ofParameter<float> areaX1, areaX2, areaY1, areaY2;
    ofParameter<float> boundsX1, boundsX2, boundsY1, boundsY2;
    ofParameter<float> epsilon;
    ofParameter<int> minPoints;
    
    ofParameter<string> oscSenderAddress;
    ofParameter<int> oscSenderPort;
    ofParameter<bool> normalizeBlobs;

    ofxOscSender oscSender;
};
