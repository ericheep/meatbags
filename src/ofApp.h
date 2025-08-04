#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxDropdown.h"

#include "OrbbecPulsar.hpp"
#include "Hokuyo.hpp"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "MeatbagsFactory.hpp"
#include "Viewer.hpp"
#include "Space.h"
#include "Sensors.hpp"
#include "Filter.hpp"
#include "Filters.hpp"
#include "OscSender.hpp"
#include "OscSenders.hpp"
#include "UI.hpp"
#include "MemoryFont.hpp"
#include "InterfaceSelector.hpp"

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
    void setupFilterGuis();
    void setupOscSenderGuis();
    void setupMeatbagsGuis();
    void setupSensorGuis();
    
    // gui positions
    void setLeftSideGuiPositions();
    void setRightSideGuiPositions();
    
    // hokuyo parameters
    void setPositionX(float &x);
    void setPositionY(float &y);
    void setAutoReconnect(bool &autoReconnectActive);
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
    void addFilter(int numberPoints);
    void removeFilter();
    void setNumberFilters(int& numberFilters);
    
    // osc senders
    void addOscSender();
    void removeOscSender();
    void setNumberOscSenders(int& numberSenders);
    
    // view parameters
    void setAreaSize(float &areaSize);

    UI buttonUI;
    Sensors sensors;
    Filters filters;
    vector<Blob> blobs;
    MeatbagsFactory meatbags;
    Space space;
    Viewer viewer;
    OscSenders oscSenders;
    InterfaceSelector interfaceSelector;
    
    ofColor guiBackgroundColor, guiBarColor, guiHeaderColor, guiTextColor;
    ofxPanel hiddenGui;
    ofxPanel generalGui;
    ofxPanel filtersGui;
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
    ofParameter<float> epsilon;
    ofParameter<int> minPoints;
    ofParameter<float> blobPersistence;
    ofParameter<bool> headlessMode;
    ofParameter<bool> autoSave;
    ofParameter<ofPoint> translation;
    ofParameter<string> localIPAddress;
    
    // viewer parameters
    ofParameter<float> areaSize;
    float saveNotificationTimer, saveNotificationTotalTime;
    bool moveActive, ctrlKeyActive, isHelpMode;
    string version;
};
