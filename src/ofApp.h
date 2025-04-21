#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxOsc.h"

#include "Hokuyo.hpp"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "Viewer.hpp"

class ofApp : public ofBaseApp{
    
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void windowResized(int width, int height) override;

    void drawFps();
    
    void setBoundsX1(float &boundsX1);
    void setBoundsX2(float &boundsX2);
    void setBoundsY1(float &boundsY1);
    void setBoundsY2(float &boundsY2);

    void setAutoReconnect(bool &autoReconnectActive);
    void setSensorMotorSpeed(int &sensorMotorSpeed);
    void setSensorRotation(float &sensorRotation);
    void setAreaSize(float &areaSize);
    void setMirrorX(bool &mirrorX);
    void setBlobPersistence(float &blobPersistence);
    void setEpsilon(float &epsilon);
    void setMinPoints(int &minPoints);
    void setOscSenderAddress(string &oscSenderAddress);
    void setOscSenderPort(int &oscSenderAddress);
    
    void sendBlobOsc();

    Hokuyo hokuyo;
    vector <Blob> blobs;
    Meatbags meatbags;
    Bounds bounds;
    Viewer viewer;
    
    ofTrueTypeFont globalFont;

    ofxPanel gui;
    ofParameterGroup meatbagsSettings;
    ofParameter<float> areaSize;
    ofParameter<bool> mirrorX;
    ofParameter<float> epsilon;
    ofParameter<int> minPoints;
    ofParameter<float> boundsX1;
    ofParameter<float> boundsX2;
    ofParameter<float> boundsY1;
    ofParameter<float> boundsY2;
    ofParameter<float> blobPersistence;
    
    ofParameterGroup sensorSettings;
    ofParameter<int> sensorMotorSpeed;
    ofParameter<float> sensorRotation;
    ofParameter<bool> autoReconnectActive;
    ofParameter<bool> showSensorInformation;

    ofParameterGroup oscSettings;
    ofParameter<string> oscSenderAddress;
    ofParameter<int> oscSenderPort;
    ofParameter<bool> normalizeBlobs;
    ofParameter<bool> oscActive;
    
    ofxOscSender oscSender;
};
