#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"

#include "Hokuyo.hpp"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "Viewer.hpp"
#include "Space.h"
#include "Sensors.hpp"
#include "Filter.hpp"
#include "Filters.hpp"
#include "OscSender.hpp"
#include "OscSenders.hpp"
#include "UI.hpp"

#define VERSION "v0.1.2"

class ofApp : public ofBaseApp{
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void windowResized(int width, int height) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    void keyPressed(int key) override;

    void drawFps();
    void save();
    
    // setup gui
    void setupGui();
    void setupFilterGuis();
    void setupOscSenderGuis();
    void setupSensorGuis();
    void setRightSideGuiPositions();
    
    // hokuyo parameters
    void setPositionX(float &x);
    void setPositionY(float &y);
    void setAutoReconnect(bool &autoReconnectActive);
    void setSensorRotation(float &sensorRotation);
    void setMirrorAngles(bool &mirrorAngles);
    
    // bounds parameters
    void updateGuiBounds();
    void setSpace();
    
    // sensors
    void addSensor();
    void removeSensor();
    void setNumberSensors(int & numberSensors);
    
    // filterss
    void addFilter(int numberPoints);
    void removeFilter();
    void setNumberFilters(int & numberFilters);
    
    // osc senders
    void addOscSender();
    void removeOscSender();
    void setNumberOscSenders(int & numberSenders);
    
    // view parameters
    void setAreaSize(float &areaSize);

    UI buttonUI;
    Sensors sensors;
    Filters filters;
    vector<Blob> blobs;
    Meatbags meatbags;
    Space space;
    Viewer viewer;
    OscSenders oscSenders;
    
    ofxPanel hiddenGui;;
    ofxPanel meatbagsGui;
    ofxPanel filtersGui;
    ofxPanel oscSendersGui;

    vector<ofxPanel*> sensorGuis;
    vector<ofxPanel*> filterGuis;
    vector<ofxPanel*> oscSenderGuis;

    ofPoint origin;
    
    // meatbags parameters
    ofParameterGroup meatbagsSettings;
    ofParameter<float> epsilon;
    ofParameter<int> minPoints;
    ofParameter<float> blobPersistence;
    ofParameter<bool> headlessMode;
    
    // viewer parameters
    ofParameter<float> areaSize;
};
