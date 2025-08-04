//
//  SensorGUIManager.hpp
//  meatbags
//

#ifndef SensorGUIManager_hpp
#define SensorGUIManager_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxDropdown.h"
#include "Sensor.hpp"
#include "Hokuyo.hpp"
#include "OrbbecPulsar.hpp"

class SensorGUIManager {
public:
    struct SensorGUIInfo {
        unique_ptr<Sensor> sensor;
        unique_ptr<ofxPanel> gui;
        int index;
    };
    
private:
    vector<SensorGUIInfo> sensorGUIs;
    ofColor guiBackgroundColor;
    ofColor guiTextColor;
    
public:
    SensorGUIManager();
    
    void addSensor(const string& sensorType);
    void changeSensorType(int index, const string& newType);
    
    void update();
    void draw();
    
    vector<Sensor*> getSensors();
    
private:
    unique_ptr<Sensor> createSensorOfType(const string& type);
    unique_ptr<ofxPanel> createGUIForSensor(Sensor* sensor, const string& sensorType);
    
    void setupCommonParameters(ofxPanel* gui, Sensor* sensor);
    void setupHokuyoParameters(ofxPanel* gui, Hokuyo* hokuyo);
    void setupOrbbecParameters(ofxPanel* gui, OrbbecPulsar* orbbec);
    
    struct SensorConfig {
        string ipAddress;
        float positionX, positionY, rotation;
        ofColor color;
        bool autoReconnect, mirrorAngles;
        int whichMeatbag;
        bool showInfo;
    };
    
    SensorConfig saveConfiguration(Sensor* sensor);
    void restoreConfiguration(Sensor* sensor, const SensorConfig& config);
};

#endif /* SensorGUIManager_hpp */
