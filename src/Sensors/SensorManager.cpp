//
//  SensorManager.cpp
//  meatbags
//

#include "SensorManager.hpp"

SensorManager::SensorManager() {
    lidarPoints.resize(21600);
    numberLidarPoints = 0;
}

SensorManager::~SensorManager() {
    clear();
}

void SensorManager::setInterfaceAndIP(string interface, string IP) {
    for (auto &entry : sensorEntries) {
        entry.sensor->interface = interface;
        entry.sensor->localIPAddress = IP;
    }
}

void SensorManager::start() {
    hasStarted = true;
}

void SensorManager::update() {
    lastFrameTime = ofGetLastFrameTime();
    
    for (auto &entry : sensorEntries) {
        entry.sensor->lastFrameTime = lastFrameTime;
        entry.sensor->update();
    }
    
    for (int i = 0; i < sensorEntries.size(); ++i) {
        auto& entry = sensorEntries[i];
        if (entry.nextType.empty()) continue;
        
        std::string type = entry.nextType;
        entry.nextType.clear();
        
        SensorType newSensorType = stringToSensorType(type);
        SensorType currentType = getCurrentSensorType(entry.sensor.get());
        
        if (newSensorType != currentType) {
            changeSensorType(i, newSensorType);
            refreshGUIPositions();
        }
    }
}

void SensorManager::draw() {
    for (auto& entry : sensorEntries) {
        if (entry.sensor) entry.sensor->draw();
        if (entry.gui) entry.gui->draw();
    }
}

void SensorManager::clear() {
    for (auto& entry : sensorEntries) {
        if (entry.sensor) {
            entry.sensor->sensorTypes.removeListener(this, &SensorManager::onSensorTypeChanged);
        }
    }
    sensorEntries.clear();
}

void SensorManager::saveTo(ofJson& config) {
    config["number_sensors"] = sensorEntries.size();
    for (auto& entry : sensorEntries) {
        entry.gui->saveTo(config);
    }
}

void SensorManager::loadSensors(int numberFilters, ofJson& config) {
    for (int i = 0; i < numberFilters; i++) {
        addSensor();
    }
    for (int i = 0; i < sensorEntries.size(); i++) {
        string sensorKey = "sensor_" + to_string(i + 1);
        ofJson sensorConfig;
        sensorConfig[sensorKey] = config[sensorKey];
        sensorEntries[i].gui->loadFrom(sensorConfig);
    }
}

void SensorManager::initialize() {
    addSensor();
}

void SensorManager::load(ofJson& config) {
    if (config.contains("number_sensors")) {
        int numberSensors = config["number_sensors"];
        loadSensors(numberSensors, config);
    } else {
        addSensor();
    }
}

void SensorManager::addSensor() {
    if (sensorEntries.size() < 6) {
        addSensor(SensorType::OrbbecPulsar);
    }
}

void SensorManager::addSensor(SensorType type) {
    auto sensor = createSensorOfType(type);
    
    if (sensor) sensor->index = sensorEntries.size() + 1;
    
    ofPoint center = ofPoint(0, 1.25);
    
    float centerRatio = float(sensor->index - 1) / 6.0;
    float sensorX = cos(centerRatio * TWO_PI - HALF_PI) * 1.25 + center.x;
    float sensorY = sin(centerRatio * TWO_PI - HALF_PI) * 1.25 + center.y;
    
    sensor->positionX = sensorX;
    sensor->positionY = sensorY;
    
    auto gui = createGUIForSensor(sensor.get(), type);
    
    SensorEntry entry;
    entry.index = sensor->index;
    entry.sensor = std::move(sensor);
    entry.gui = std::move(gui);
    
    sensorEntries.push_back(std::move(entry));
    refreshGUIPositions();
}

void SensorManager::removeSensor() {
    if (sensorEntries.size() > 1) {
        sensorEntries.back().sensor->sensorTypes.removeListener(this, &SensorManager::onSensorTypeChanged);
        
        sensorEntries.pop_back();
    }
}

void SensorManager::changeSensorType(int index, SensorType newType) {
    if (index < 0 || index >= sensorEntries.size()) return;
    
    auto& entry = sensorEntries[index];
    
    SensorType currentType = getCurrentSensorType(entry.sensor.get());
    if (currentType == newType) return;
    
    Sensor* oldSensor = entry.sensor.get();
    
    int savedIndex = oldSensor->index;
    ofColor savedColor = oldSensor->sensorColor;
    string savedIP = oldSensor->ipAddress;
    float savedPosX = oldSensor->positionX;
    float savedPosY = oldSensor->positionY;
    float savedRotation = oldSensor->sensorRotationDeg;
    bool savedMirror = oldSensor->mirrorAngles;
    int savedMeatbag = oldSensor->whichMeatbag;
    bool savedShowInfo = oldSensor->showSensorInformation;
    
    Space savedSpace = oldSensor->space;
    ofPoint savedTranslation = oldSensor->translation;
    
    if (oldSensor) oldSensor->sensorTypes.removeListener(this, &SensorManager::onSensorTypeChanged);
    
    auto newSensor = createSensorOfType(newType);
    if (!newSensor) return;
    
    newSensor->setupParameters();
    newSensor->initializeVectors();
    
    newSensor->index = savedIndex;
    newSensor->sensorColor = savedColor;
    newSensor->ipAddress = savedIP;
    
    newSensor->positionX.setWithoutEventNotifications(savedPosX);
    newSensor->positionY.setWithoutEventNotifications(savedPosY);
    newSensor->sensorRotationDeg.setWithoutEventNotifications(savedRotation);
    newSensor->mirrorAngles.setWithoutEventNotifications(savedMirror);
    
    newSensor->whichMeatbag = savedMeatbag;
    newSensor->showSensorInformation = savedShowInfo;
    
    newSensor->setSpace(savedSpace);
    newSensor->setTranslation(savedTranslation);
    
    auto newGUI = createGUIForSensor(newSensor.get(), newType);
    entry.sensor = std::move(newSensor);
    entry.gui = std::move(newGUI);
    /*entry.sensor->update();
     
     if (savedAutoReconnect && !savedIP.empty() && savedIP != "0.0.0.0") {
     ofLogNotice("SensorManager") << "Auto-connecting new " << sensorTypeToString(newType)
     << " sensor to " << savedIP << " (using default port for this sensor type)";
     entry.sensor->connect();
     }
     
     entry.sensor->showSensorInformation = false;*/
    refreshGUIPositions();
}

vector<Sensor*> SensorManager::getSensors() {
    vector<Sensor*> sensors;
    for (auto& entry : sensorEntries) {
        sensors.push_back(entry.sensor.get());
    }
    return sensors;
}

void SensorManager::transferSensorState(Sensor* oldSensor, Sensor* newSensor) {
    if (!oldSensor || !newSensor) return;
    
    newSensor->index = oldSensor->index;
    newSensor->sensorColor = oldSensor->sensorColor;
    newSensor->ipAddress = oldSensor->ipAddress;
    newSensor->positionX = oldSensor->positionX;
    newSensor->positionY = oldSensor->positionY;
    newSensor->sensorRotationDeg = oldSensor->sensorRotationDeg;
    newSensor->mirrorAngles = oldSensor->mirrorAngles;
    newSensor->whichMeatbag = oldSensor->whichMeatbag;
    newSensor->showSensorInformation = oldSensor->showSensorInformation;
    
    newSensor->update();
}

SensorType SensorManager::getCurrentSensorType(Sensor* sensor) {
    if (dynamic_cast<Hokuyo*>(sensor)) {
        return SensorType::Hokuyo;
    } else if (dynamic_cast<OrbbecPulsar*>(sensor)) {
        return SensorType::OrbbecPulsar;
    }
    return SensorType::OrbbecPulsar;
}

SensorType SensorManager::stringToSensorType(const std::string& typeStr) {
    if (typeStr == "Hokuyo") {
        return SensorType::Hokuyo;
    } else if (typeStr == "Orbbec Pulsar") {
        return SensorType::OrbbecPulsar;
    }
    return SensorType::OrbbecPulsar;
}

string SensorManager::sensorTypeToString(SensorType type) {
    switch (type) {
        case SensorType::Hokuyo: return "Hokuyo";
        case SensorType::OrbbecPulsar: return "Orbbec Pulsar";
        default: return "Hokuyo";
    }
}

void SensorManager::setFilters(const std::vector<Filter*>& filters_) {
    filters = filters_;
}

void SensorManager::refreshGUIPositions() {
    int yOffset = 313;
    int nextYPos = 0.0;
    
    for (int i = 0; i < sensorEntries.size(); ++i) {
        auto& entry = sensorEntries[i];
        if (!(entry.gui && entry.sensor)) continue;
        
        int guiHeight = 0;
        int margin = 10;
        int xPos = 0;
        int yPos = 0;
        
        
        if (i == 2) nextYPos = 0;
        
        if (i < 2) {
            yOffset = 313;
            xPos = margin;
        } else {
            yOffset = 135;
            xPos = 200 + margin + (margin * 0.5);
        }
        
        yPos = nextYPos + yOffset;
        entry.gui->setPosition(xPos, yPos);
        entry.sensor->setInfoPosition(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
        
        string type = sensorTypeToString(getCurrentSensorType(entry.sensor.get()));
        if (type == "Hokuyo") {
            guiHeight = 120;
        } else if (type == "Orbbec Pulsar") {
            guiHeight = 145;
        }
        
        nextYPos += guiHeight + margin + 5;
    }
}

unique_ptr<Sensor> SensorManager::createSensorOfType(SensorType type) {
    switch (type) {
        case SensorType::Hokuyo:
            return make_unique<Hokuyo>();
        case SensorType::OrbbecPulsar:
            return make_unique<OrbbecPulsar>();
        default:
            return make_unique<Hokuyo>();
    }
}

unique_ptr<ofxPanel> SensorManager::createGUIForSensor(Sensor* sensor, SensorType sensorType) {
    auto gui = make_unique<ofxPanel>();
    
    gui->setDefaultWidth(200);
    gui->setup("sensor " + to_string(sensor->index));
    
    setupCommonParameters(gui.get(), sensor);
    
    switch (sensorType) {
        case SensorType::Hokuyo:
            setupHokuyoParameters(gui.get(), static_cast<Hokuyo*>(sensor));
            break;
        case SensorType::OrbbecPulsar:
            setupOrbbecParameters(gui.get(), static_cast<OrbbecPulsar*>(sensor));
            break;
    }
    
    return gui;
}

void SensorManager::setupCommonParameters(ofxPanel* gui, Sensor* sensor) {
    if (!gui || !sensor) return;
    
    int currentIndex = sensorEntries.size();
    float hue = fmod((sensor->index - 1) * 31.875 + 140.0, 255);
    ofColor sensorColor = ofColor::fromHsb(hue, 125.0, 255.0);
    ofColor backgroundColor = ofColor::fromHsb(hue, 60.0, 255);
    
    SensorType currentType = getCurrentSensorType(sensor);
    sensor->sensorTypes.selectedValue = sensorTypeToString(currentType);
    
    if (sensor->whichMeatbag == 0) sensor->whichMeatbag = 1;
    
    gui->setDefaultBackgroundColor(backgroundColor);
    gui->setBackgroundColor(backgroundColor);
    gui->setHeaderBackgroundColor(sensorColor);
    gui->setFillColor(sensorColor);
    gui->setDefaultFillColor(sensorColor);
    
    gui->add(&sensor->sensorTypes);
    sensor->sensorTypes.setBackgroundColor(backgroundColor);
    sensor->sensorTypes.setDefaultFillColor(backgroundColor);
    sensor->sensorTypes.setDefaultBackgroundColor(backgroundColor);
    sensor->sensorTypes.setDefaultHeaderBackgroundColor(backgroundColor);
    sensor->sensorTypes.setHeaderBackgroundColor(sensorColor);
    sensor->sensorTypes.setFillColor(sensorColor);
    
    gui->add(sensor->ipAddress.set("IP address", sensor->ipAddress));
    gui->add(sensor->whichMeatbag.set("which meatbag", sensor->whichMeatbag, 1, 2));
    gui->add(sensor->mirrorAngles.set("mirror angles", sensor->mirrorAngles));
    gui->add(sensor->showSensorInformation.set("show sensor info", sensor->showSensorInformation));
    gui->add(sensor->positionX.set("position x", sensor->positionX, -15.0, 15.0));
    gui->add(sensor->positionY.set("position y", sensor->positionY, 0.0, 30.0));
    gui->add(sensor->sensorRotationDeg.set("sensor rotation (deg)", sensor->sensorRotationDeg, -90.0, 270.0));
    
    sensor->sensorColor = sensorColor;
    sensor->setInfoPosition(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
    sensor->showSensorInformation = false;
    
    sensor->update();
    sensor->sensorTypes.addListener(this, &SensorManager::onSensorTypeChanged);
}

bool SensorManager::areNewCoordinatesAvailable() {
    bool newCoordinatesAvalable = false;
    for (auto& entry : sensorEntries) {
        if (entry.sensor->newCoordinatesAvailable) {
            newCoordinatesAvalable = true;
            entry.sensor->newCoordinatesAvailable = false;
            break;
        }
    }
    return newCoordinatesAvalable;}

void SensorManager::setupHokuyoParameters(ofxPanel* gui, Hokuyo* hokuyo) {
    if (!hokuyo) return;
    // nothing unique here for now
}

void SensorManager::setupOrbbecParameters(ofxPanel* gui, OrbbecPulsar* orbbec) {
    if (!orbbec) return;
    
    gui->add(orbbec->guiMotorSpeed.set("rotation speed", 1, 1, 4));
    gui->add(orbbec->guiSpecialWorkingMode.set("fog mode", false));
}

void SensorManager::onSensorTypeChanged(string& selectedType) {
    for (int i = 0; i < sensorEntries.size(); ++i) {
        auto& entry = sensorEntries[i];
        if (!entry.sensor) continue;
        
        string currentTypeName = sensorTypeToString(getCurrentSensorType(entry.sensor.get()));
        string dropdownValue = entry.sensor->sensorTypes.selectedValue;
        
        if (dropdownValue == selectedType && currentTypeName != selectedType) {
            entry.nextType = selectedType;
            break;
        }
    }
}

void SensorManager::setSpace(Space& space) {
    for (auto& entry : sensorEntries) {
        if (entry.sensor) entry.sensor->setSpace(space);
    }
}

void SensorManager::setTranslation(ofPoint translation) {
    for (auto& entry : sensorEntries) {
        if (entry.sensor) entry.sensor->setTranslation(translation);
    }
}

bool SensorManager::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    for (auto& entry : sensorEntries) {
        if (entry.sensor && entry.sensor->onMouseMoved(mouseArgs)) return true;
    }
    return false;
}

bool SensorManager::onMousePressed(ofMouseEventArgs& mouseArgs) {
    for (auto& entry : sensorEntries) {
        if (entry.sensor && entry.sensor->onMousePressed(mouseArgs)) return true;
    }
    return false;
}

bool SensorManager::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    for (auto& entry : sensorEntries) {
        if (entry.sensor && entry.sensor->onMouseDragged(mouseArgs)) return true;
    }
    return false;
}

bool SensorManager::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    for (auto& entry : sensorEntries) {
        if (entry.sensor && entry.sensor->onMouseReleased(mouseArgs)) return true;
    }
    return false;
}

bool SensorManager::onKeyPressed(ofKeyEventArgs& keyArgs) {
    for (auto& entry : sensorEntries) {
        // if (entry.sensor && entry.sensor->onKeyPressed(keyArgs)) return true;
    }
    return false;
}

void SensorManager::getCoordinates(const std::vector<Meatbags*>& meatbags) {
    int overallCounter = 0;

    for (auto& meatbag : meatbags) {
        int inFilterCounter = 0;
        
        for (auto& entry : sensorEntries) {
            if (entry.sensor->whichMeatbag == meatbag->index) {
                for (auto& coordinate : entry.sensor->coordinates) {
                    float x = coordinate.x;
                    float y = coordinate.y;
                    
                    float distance = entry.sensor->position.distance(coordinate);
                    
                    // filters out if distances are 150mm too close
                    if (distance > 150) {
                        overallCounter++;
                        
                        lidarPoints[overallCounter].coordinate.set(x, y);
                        lidarPoints[overallCounter].color = entry.sensor->sensorColor;
                        lidarPoints[overallCounter].isInFilter = false;
                        
                        if (checkWithinFilters(x, y)) {
                            meatbag->coordinates[inFilterCounter].set(x, y);
                            inFilterCounter++;
                            
                            lidarPoints[overallCounter].isInFilter = true;
                        }
                    }
                }
            }
        }
        
        meatbag->numberCoordinates = inFilterCounter;
    }
    
    numberLidarPoints = overallCounter;
}

bool SensorManager::checkWithinFilters(float x, float y) {
    bool isWithinFilter = false;
    for (const auto &filter : filters) {
        if (filter->checkInside(x * 0.001, y * 0.001)) {
            if (filter->isMask) {
                return false;
            } else {
                isWithinFilter = true;
            }
        }
    }
    
    return isWithinFilter;
}
