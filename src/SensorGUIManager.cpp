//
//  SensorGUIManager.cpp
//  meatbags
//

#include "SensorGUIManager.hpp"

SensorGUIManager::SensorGUIManager() {
    guiBackgroundColor = ofColor::darkGray;
    guiTextColor = ofColor::white;
}

void SensorGUIManager::addSensor(const string& sensorType = "Hokuyo") {
    SensorGUIInfo info;
    info.index = sensorGUIs.size() + 1;
    
    // Create sensor using factory
    info.sensor = createSensorOfType(sensorType);
    info.sensor->index = info.index;
    
    // Create GUI for this sensor
    info.gui = createGUIForSensor(info.sensor.get(), sensorType);
    
    sensorGUIs.push_back(std::move(info));
}

void SensorGUIManager::changeSensorType(int index, const string& newType) {
    if (index >= sensorGUIs.size()) return;
    
    auto& info = sensorGUIs[index];
    
    // Save current configuration
    auto config = saveConfiguration(info.sensor.get());
    
    // Create new sensor
    info.sensor = createSensorOfType(newType);
    info.sensor->index = info.index;
    
    // Restore configuration
    restoreConfiguration(info.sensor.get(), config);
    
    // Recreate GUI for new sensor type
    info.gui = createGUIForSensor(info.sensor.get(), newType);
}

void SensorGUIManager::update() {
    for (auto& info : sensorGUIs) {
        info.sensor->update();
    }
}

void SensorGUIManager::draw() {
    for (auto& info : sensorGUIs) {
        info.sensor->draw();
        info.gui->draw();
    }
}

vector<Sensor*> SensorGUIManager::getSensors() {
    vector<Sensor*> sensors;
    for (auto& info : sensorGUIs) {
        sensors.push_back(info.sensor.get());
    }
    return sensors;
}

unique_ptr<Sensor> SensorGUIManager::createSensorOfType(const string& type) {
    if (type == "Hokuyo") {
        return make_unique<Hokuyo>();
    } else if (type == "Orbbec Pulsar") {
        return make_unique<OrbbecPulsar>();
    }
    return make_unique<Hokuyo>();
}

unique_ptr<ofxPanel> SensorGUIManager::createGUIForSensor(Sensor* sensor, const string& sensorType) {
    auto gui = make_unique<ofxPanel>();
    
    // Basic panel setup
    gui->setFillColor(ofColor::thistle);
    gui->setBackgroundColor(guiBackgroundColor);
    gui->setTextColor(guiTextColor);
    gui->setDefaultWidth(190);
    gui->setup("sensor " + to_string(sensor->index));
    
    // Common parameters
    setupCommonParameters(gui.get(), sensor);
    
    // Type-specific parameters
    if (sensorType == "Hokuyo") {
        setupHokuyoParameters(gui.get(), static_cast<Hokuyo*>(sensor));
    } else if (sensorType == "Orbbec Pulsar") {
        setupOrbbecParameters(gui.get(), static_cast<OrbbecPulsar*>(sensor));
    }
    
    return gui;
}

void SensorGUIManager::setupCommonParameters(ofxPanel* gui, Sensor* sensor) {
    // Color
    int currentIndex = sensor->index - 1;
    float hue = fmod(currentIndex * 31.875 + 140.0, 255);
    ofColor randomColor = ofColor::fromHsb(hue, 125.0, 255.0);
    gui->add(sensor->sensorColor.set("color", randomColor));
    
    // Sensor type dropdown
    vector<string> sensorTypes = {"Hokuyo", "Orbbec Pulsar"};
    sensor->typesDropdown.add(sensorTypes);
    gui->add(&sensor->typesDropdown);
    
    // Network settings
    gui->add(sensor->ipAddress.set("IP address", "0.0.0.0"));
    gui->add(sensor->autoReconnectActive.set("auto reconnect", true));
    
    // Position settings
    ofPoint center = ofPoint(0, 1.25);
    float centerRatio = float(currentIndex) / 8.0;
    float sensorX = cos(centerRatio * TWO_PI - HALF_PI) * 1.25 + center.x;
    float sensorY = sin(centerRatio * TWO_PI - HALF_PI) * 1.25 + center.y;
    
    gui->add(sensor->positionX.set("position x", sensorX, -15.0, 15.0));
    gui->add(sensor->positionY.set("position y", sensorY, 0.0, 30.0));
    gui->add(sensor->sensorRotationDeg.set("sensor rotation (deg)", 0, -180.0, 180.0));
    
    // Display settings
    gui->add(sensor->mirrorAngles.set("mirror angles", false));
    gui->add(sensor->whichMeatbag.set("which meatbag", 1, 1, 2));
    gui->add(sensor->showSensorInformation.set("show sensor info", false));
    
    sensor->setInfoPosition(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
}

void SensorGUIManager::setupHokuyoParameters(ofxPanel* gui, Hokuyo* hokuyo) {
    // Add Hokuyo-specific parameters here
    // For example:
    // gui->add(hokuyo->laserIntensity.set("laser intensity", 100, 0, 255));
    // gui->add(hokuyo->scanRate.set("scan rate", 10, 1, 40));
}

void SensorGUIManager::setupOrbbecParameters(ofxPanel* gui, OrbbecPulsar* orbbec) {
    // Add Orbbec-specific parameters here
    // For example:
    // gui->add(orbbec->rotationSpeed.set("rotation speed", 15, 15, 40));
    // gui->add(orbbec->filterLevel.set("filter level", 0, 0, 5));
}

void SensorGUIManager::restoreConfiguration(Sensor* sensor, const SensorConfig& config) {
    sensor->ipAddress.set(config.ipAddress);
    sensor->positionX.set(config.positionX);
    sensor->positionY.set(config.positionY);
    sensor->sensorRotationDeg.set(config.rotation);
    sensor->sensorColor.set(config.color);
    sensor->autoReconnectActive.set(config.autoReconnect);
    sensor->mirrorAngles.set(config.mirrorAngles);
    sensor->whichMeatbag.set(config.whichMeatbag);
    sensor->showSensorInformation.set(config.showInfo);
}
