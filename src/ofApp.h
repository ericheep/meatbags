#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"

#include "Hokuyo.hpp"
#include "Meatbags.hpp"

class ofApp : public ofBaseApp{
    
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void drawFps();
    
    Hokuyo hokuyo;
    
    // blob tracking software
    Meatbags meatbags;
    
    ofxPanel gui;
    ofParameter<float> scale;
    ofParameter<float> areaX1, areaX2, areaY1, areaY2;
    ofParameter<float> boundsX1, boundsX2, boundsY1, boundsY2;
    ofParameter<float> epsilon;
    ofParameter<int> minPoints;
};
