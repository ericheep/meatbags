//
//  SensorManager.hpp
//  meatbags
//

#ifndef SensorManager_hpp
#define SensorManager_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxDropdown.h"
#include "Sensor.hpp"
#include "Hokuyo.hpp"
#include "OrbbecPulsar.hpp"
#include "Space.h"
#include "Filter.hpp"
#include "MeatbagsManager.hpp"

enum class SensorType {
    Hokuyo,
    OrbbecPulsar
};

class SensorManager {
public:
    struct SensorEntry {
        int index;
        unique_ptr<Sensor> sensor;
        unique_ptr<ofxPanel> gui;
        std::string nextType;
    };
    
private:
    vector<SensorEntry> sensorEntries;
    ofColor guiBackgroundColor;
    ofColor guiTextColor;
    
public:
    SensorManager();
    ~SensorManager();
    
    void update();
    void draw();
    void clear();
    void saveTo(ofJson& configuration);
    void load(ofJson configuration);
    void loadSensors(int numberSensors, ofJson configuration);
    void setInterfaceAndIP(string interface, string IP);
    
    void addSensor();
    void addSensor(SensorType type);
    void changeSensorType(int index, SensorType newType);
    void removeSensor();
    vector<Sensor*> getSensors();

    void setFilters(const std::vector<Filter*>& filters);
    void setSpace(Space& space);
    void setTranslation(ofPoint translation);
    
    bool checkWithinFilters(float x, float y);
    void getCoordinates(const std::vector<Meatbags*>& meatbags);
    
    bool onMouseMoved(ofMouseEventArgs& mouseArgs);
    bool onMousePressed(ofMouseEventArgs& mouseArgs);
    bool onMouseDragged(ofMouseEventArgs& mouseArgs);
    bool onMouseReleased(ofMouseEventArgs& mouseArgs);
    bool onKeyPressed(ofKeyEventArgs& keyArgs);
    
    bool areNewCoordinatesAvailable();

    void refreshGUIPositions();
    void start();
private:
    unique_ptr<Sensor> createSensorOfType(SensorType type);
    unique_ptr<ofxPanel> createGUIForSensor(Sensor* sensor, SensorType type);

    void setupCommonParameters(ofxPanel* gui, Sensor* sensor);
    void setupHokuyoParameters(ofxPanel* gui, Hokuyo* hokuyo);
    void setupOrbbecParameters(ofxPanel* gui, OrbbecPulsar* orbbec);
    
    void onSensorTypeChanged(string& selectedType);
    void transferSensorState(Sensor* oldSensor, Sensor* newSensor);
    SensorType getCurrentSensorType(Sensor* sensor);
    SensorType stringToSensorType(const std::string& typeStr);
    string sensorTypeToString(SensorType type);
    
    float lastFrameTime;
    vector<Filter*> filters;
    bool hasStarted = false;
};

#endif /* SensorManager_hpp */
