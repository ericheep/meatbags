//
//  SensorManager.hpp
//  meatbags

#ifndef SensorManager_hpp
#define SensorManager_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Sensor.hpp"
#include "Hokuyo.hpp"
#include "OrbbecPulsar.hpp"
#include "Space.h"
#include "Filter.hpp"
#include "Meatbags.hpp"
#include "LidarPoint.h"

enum class SensorType {
    Hokuyo,
    OrbbecPulsar
};

class SensorManager {
public:
    struct SensorEntry {
        int index;
        unique_ptr<Sensor> sensor;
        std::string nextType;
    };

    SensorManager();
    ~SensorManager();

    void update();
    void draw();
    void clear();

    void initialize();
    void saveTo(ofJson& configuration);
    void load(ofJson& configuration);
    void loadSensors(int numberSensors, ofJson& configuration);
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
    void start();

    SensorType stringToSensorType(const std::string& typeStr);

    vector<LidarPoint> lidarPoints;
    int numberLidarPoints;

private:
    vector<SensorEntry> sensorEntries;

    unique_ptr<Sensor> createSensorOfType(SensorType type);
    void transferSensorState(Sensor* oldSensor, Sensor* newSensor);
    SensorType getCurrentSensorType(Sensor* sensor);
    string     sensorTypeToString(SensorType type);
    ofColor    sensorColorForIndex(int index);

    float lastFrameTime;
    vector<Filter*> filters;
    bool hasStarted = false;
};

#endif /* SensorManager_hpp */