#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxOsc.h"

#include "Hokuyo.hpp"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "Viewer.hpp"

class SensorParameterFunction {
public:
    Hokuyo* hokuyo = NULL;
    
    SensorParameterFunction(Hokuyo * _hokuyo) {
        hokuyo = _hokuyo;
    }
    
    void setIPAddress(string &ipAddress) {
        hokuyo->setIPAddress(ipAddress);
    }
};

class ofApp : public ofBaseApp{
public:
    
    
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void windowResized(int width, int height) override;
    
    void drawFps();
    
    // hokuyo parameters
    // void setIPAddress(string &ipAddress);
    void setPositionX(float &x);
    void setPositionY(float &y);
    void setAutoReconnect(bool &autoReconnectActive);
    void setSensorRotation(float &sensorRotation);
    void setMirrorAngles(bool &mirrorAngles);
    
    // bounds parameters
    void updateGuiBounds();
    void setBoundsX1(float &boundsX1);
    void setBoundsX2(float &boundsX2);
    void setBoundsY1(float &boundsY1);
    void setBoundsY2(float &boundsY2);
    
    // view parameters
    void setAreaSize(float &areaSize);
    
    // meatbags parameters
    void setBlobPersistence(float &blobPersistence);
    void setEpsilon(float &epsilon);
    void setMinPoints(int &minPoints);
    
    // osc parameters
    void setOscSenderAddress(string &oscSenderAddress);
    void setOscSenderPort(int &oscSenderAddress);
    
    void sendBlobOsc();
    
    vector<Hokuyo *> hokuyos;
    vector<Blob> blobs;
    Meatbags meatbags;
    Bounds bounds;
    Viewer viewer;
    ofxOscSender oscSender;
    
    ofxPanel gui;
    
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
    
    // hokuyo parameters
    vector<ofParameterGroup> sensorSettings;
    vector<ofParameter<string>> sensorIPAddresses;
    vector<ofParameter<float>> positionXs;
    vector<ofParameter<float>> positionYs;
    vector<ofParameter<bool>> autoReconnectsActive;
    vector<ofParameter<bool>> mirrorAngles;
    vector<ofParameter<float>> sensorRotations;
    vector<ofParameter<bool>> showSensorInformations;
    
    // osc parameters
    ofParameterGroup oscSettings;
    ofParameter<string> oscSenderAddress;
    ofParameter<int> oscSenderPort;
    ofParameter<bool> normalizeBlobs;
    ofParameter<bool> oscActive;
};
