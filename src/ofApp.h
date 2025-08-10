#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxDropdown.h"
#include "ofJson.h"

#include "SensorManager.hpp"
#include "FilterManager.hpp"
#include "MeatbagsManager.hpp"
#include "OscSenderManager.hpp"
#include "Viewer.hpp"

#include "UI.hpp"
#include "InterfaceSelector.hpp"
#include "Space.h"

class ofApp : public ofBaseApp{
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void windowResized(int width, int height) override;
    
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
    void onMouseScrolled(ofMouseEventArgs & mouseArgs);
    void onKeyPressed(ofKeyEventArgs & keyArgs);
    void onKeyReleased(ofKeyEventArgs & keyArgs);
    
    void drawMeatbags();
    void drawHelpText();
    void drawSaveNotification();
    
    void drawFps();
    void save();
    void hideWindow();
    
    // setup gui
    void setupGui();
    void setupOscSenderGuis();
    void setupMeatbagsGuis();
    
    // gui positions
    void setLeftSideGuiPositions();
    // void setRightSideGuiPositions();
    
    // sensor parameters
    void setPositionX(float &x);
    void setPositionY(float &y);
    void setSensorRotation(float &sensorRotation);
    void setMirrorAngles(bool &mirrorAngles);
    void setInterface(string &interface);
    
    // bounds parameters
    void updateGuiBounds();
    void setSpace();
    void setTranslation();
    
    void addMeatbag();
    void removeMeatbag();
    void setNumberMeatbags(int& numberMeatbags);
    
    // sensors
    void addSensor();
    void removeSensor();
    void setNumberSensors(int& numberSensors);
    
    // filterss
    void addFilter();
    void removeFilter();
    void setNumberFilters(int& numberFilters);
    
    // osc senders
    void addOscSender();
    void removeOscSender();
    void setNumberOscSenders(int& numberSenders);
    
    // view parameters
    void setAreaSize(float &areaSize);

    SensorManager sensorManager;
    FilterManager filterManager;
    OscSenderManager oscSenderManager;
    MeatbagsManager meatbagsManager;
    
    vector<Blob> blobs;
    Viewer viewer;

    InterfaceSelector interfaceSelector;
    
    UI buttonUI;
    Space space;
    
    ofColor guiBackgroundColor, guiBarColor, guiHeaderColor, guiTextColor;
    ofxPanel hiddenGui;
    ofxPanel generalGui;
    ofxPanel oscSendersGui;

    vector<ofxPanel*> meatbagsGuis;
    vector<ofxPanel*> sensorGuis;
    vector<ofxPanel*> filterGuis;
    vector<ofxPanel*> oscSenderGuis;
    ofxDropdown_<string> interfacesDropdown { "interfaces" };

    ofPoint origin, initialTranslation;
    
    // meatbags parameters
    ofParameterGroup generalSettings;
    ofParameterGroup meatbagsSettings;

    ofParameter<bool> headlessMode;
    ofParameter<ofPoint> translation;
    ofParameter<string> localIPAddress;
    
    // viewer parameters
    ofParameter<float> areaSize;
    float saveNotificationTimer, saveNotificationTotalTime;
    bool moveActive, ctrlKeyActive, isHelpMode;
    string version;
};
