//
//  SensorManager.cpp
//  meatbags

#include "SensorManager.hpp"

SensorManager::SensorManager() {
    lidarPoints.resize(21600);
    numberLidarPoints = 0;
}

SensorManager::~SensorManager() {
    clear();
}

void SensorManager::setInterfaceAndIP(string interface, string IP) {
    for (auto& entry : sensorEntries) {
        entry.sensor->interface = interface;
        entry.sensor->localIPAddress = IP;
    }
}

void SensorManager::start() {
    hasStarted = true;
}

ofColor SensorManager::sensorColorForIndex(int index) {
    float hue = fmod((index - 1) * 31.875f + 140.0f, 255.0f);
    return ofColor::fromHsb(hue, 125.0f, 255.0f);
}

void SensorManager::initialize() {
    addSensor();
}

void SensorManager::draw() {
    for (auto& entry : sensorEntries) {
        if (!entry.sensor) continue;
        // position the info overlay at the sensor's screen-space location
        ofPoint screenPos = entry.sensor->convertCoordinateToScreenPoint(entry.sensor->position);
        entry.sensor->setInfoPosition(screenPos.x, screenPos.y);
        entry.sensor->draw();
    }
}

void SensorManager::update() {
    lastFrameTime = ofGetLastFrameTime();

    for (auto& entry : sensorEntries) {
        entry.sensor->lastFrameTime = lastFrameTime;
        entry.sensor->update();
    }

    for (int i = 0; i < sensorEntries.size(); i++) {
        auto& entry = sensorEntries[i];
        if (entry.nextType.empty()) continue;

        string type = entry.nextType;
        entry.nextType.clear();

        SensorType newType = stringToSensorType(type);
        SensorType currentType = getCurrentSensorType(entry.sensor.get());
        if (newType != currentType) changeSensorType(i, newType);
    }
}

void SensorManager::clear() {
    sensorEntries.clear();
}

void SensorManager::addSensor() {
    addSensor(SensorType::OrbbecPulsar);
}

void SensorManager::addSensor(SensorType type) {
    auto sensor = createSensorOfType(type);
    sensor->index = sensorEntries.size() + 1;
    sensor->sensorColor = sensorColorForIndex(sensor->index);
    sensor->whichMeatbag = 1;

    // default position spread in circle
    ofPoint center(0, 1.25);
    float ratio = float(sensor->index - 1) / 6.0f;
    sensor->positionX = cos(ratio * TWO_PI - HALF_PI) * 1.25f + center.x;
    sensor->positionY = sin(ratio * TWO_PI - HALF_PI) * 1.25f + center.y;

    sensor->showSensorInformation = false;
    sensor->guiMotorSpeed.set("motor speed", 20, 15, 40);

    SensorEntry entry;
    entry.index = sensor->index;
    entry.sensor = std::move(sensor);
    sensorEntries.push_back(std::move(entry));
}

void SensorManager::removeSensor() {
    if (sensorEntries.size() > 1) {
        sensorEntries.pop_back();
    }
}

void SensorManager::changeSensorType(int index, SensorType newType) {
    if (index < 0 || index >= sensorEntries.size()) return;

    auto& entry = sensorEntries[index];
    SensorType currentType = getCurrentSensorType(entry.sensor.get());
    if (currentType == newType) return;

    Sensor* oldSensor = entry.sensor.get();

    int     savedIndex = oldSensor->index;
    ofColor savedColor = oldSensor->sensorColor;
    string  savedIP = oldSensor->ipAddress;
    float   savedPosX = oldSensor->positionX;
    float   savedPosY = oldSensor->positionY;
    float   savedRotation = oldSensor->sensorRotationDeg;
    bool    savedMirror = oldSensor->mirrorAngles;
    int     savedMeatbag = oldSensor->whichMeatbag;
    bool    savedShowInfo = oldSensor->showSensorInformation;
    Space   savedSpace = oldSensor->space;
    ofPoint savedTrans = oldSensor->translation;

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
    newSensor->setTranslation(savedTrans);

    entry.sensor = std::move(newSensor);
}

vector<Sensor*> SensorManager::getSensors() {
    vector<Sensor*> sensors;
    for (auto& entry : sensorEntries) {
        sensors.push_back(entry.sensor.get());
    }
    return sensors;
}

unique_ptr<Sensor> SensorManager::createSensorOfType(SensorType type) {
    switch (type) {
    case SensorType::Hokuyo:       return make_unique<Hokuyo>();
    case SensorType::OrbbecPulsar: return make_unique<OrbbecPulsar>();
    default:                       return make_unique<OrbbecPulsar>();
    }
}

SensorType SensorManager::getCurrentSensorType(Sensor* sensor) {
    if (dynamic_cast<Hokuyo*>(sensor))       return SensorType::Hokuyo;
    if (dynamic_cast<OrbbecPulsar*>(sensor)) return SensorType::OrbbecPulsar;
    return SensorType::OrbbecPulsar;
}

SensorType SensorManager::stringToSensorType(const std::string& s) {
    if (s == "Hokuyo")       return SensorType::Hokuyo;
    if (s == "Orbbec Pulsar") return SensorType::OrbbecPulsar;
    return SensorType::OrbbecPulsar;
}

string SensorManager::sensorTypeToString(SensorType type) {
    switch (type) {
    case SensorType::Hokuyo:       return "Hokuyo";
    case SensorType::OrbbecPulsar: return "Orbbec Pulsar";
    default:                       return "Orbbec Pulsar";
    }
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

void SensorManager::setFilters(const std::vector<Filter*>& filters_) {
    filters = filters_;
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

bool SensorManager::areNewCoordinatesAvailable() {
    for (auto& entry : sensorEntries) {
        if (entry.sensor->newCoordinatesAvailable) {
            entry.sensor->newCoordinatesAvailable = false;
            return true;
        }
    }
    return false;
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
    return false;
}

void SensorManager::getCoordinates(const std::vector<Meatbags*>& meatbags) {
    int overallCounter = 0;

    for (auto& meatbag : meatbags) {
        int inFilterCounter = 0;

        for (auto& entry : sensorEntries) {
            if (entry.sensor->whichMeatbag != meatbag->index) continue;

            for (auto& coordinate : entry.sensor->coordinates) {
                float x = coordinate.x;
                float y = coordinate.y;

                float distance = entry.sensor->position.distance(coordinate);
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

        meatbag->numberCoordinates = inFilterCounter;
    }

    numberLidarPoints = overallCounter;
}

bool SensorManager::checkWithinFilters(float x, float y) {
    bool isWithinFilter = false;
    for (const auto& filter : filters) {
        if (filter->checkInside(x * 0.001f, y * 0.001f)) {
            if (filter->isMask) return false;
            else isWithinFilter = true;
        }
    }
    return isWithinFilter;
}

// -----------------------------------------------------------------------------
// Save / Load
// -----------------------------------------------------------------------------

void SensorManager::saveTo(ofJson& config) {
    config["number_sensors"] = sensorEntries.size();
    for (int i = 0; i < sensorEntries.size(); i++) {
        string key = "sensor_" + to_string(i + 1);
        Sensor* s = sensorEntries[i].sensor.get();

        config[key]["type"] = sensorTypeToString(getCurrentSensorType(s));
        config[key]["ip_address"] = s->ipAddress.get();
        config[key]["position_x"] = s->positionX.get();
        config[key]["position_y"] = s->positionY.get();
        config[key]["rotation"] = s->sensorRotationDeg.get();
        config[key]["mirror"] = s->mirrorAngles.get();
        config[key]["which_meatbag"] = s->whichMeatbag.get();
        config[key]["show_info"] = s->showSensorInformation.get();

        // Orbbec-specific
        if (dynamic_cast<OrbbecPulsar*>(s)) {
            config[key]["motor_speed"] = s->guiMotorSpeed.get();  // Hz: 15/20/25/30/40
            config[key]["fog_mode"] = s->guiSpecialWorkingMode.get();
        }
    }
}

void SensorManager::loadSensors(int n, ofJson& config) {
    for (int i = 0; i < n; i++) addSensor();

    for (int i = 0; i < sensorEntries.size(); i++) {
        string key = "sensor_" + to_string(i + 1);
        if (!config.contains(key)) continue;
        ofJson& sc = config[key];
        Sensor* s = sensorEntries[i].sensor.get();

        // type swap first
        if (sc.contains("type")) {
            SensorType newType = stringToSensorType(sc["type"].get<string>());
            if (newType != getCurrentSensorType(s)) {
                changeSensorType(i, newType);
                s = sensorEntries[i].sensor.get();
            }
        }

        if (sc.contains("ip_address"))    s->ipAddress = sc["ip_address"].get<string>();
        if (sc.contains("position_x"))    s->positionX = sc["position_x"].get<float>();
        if (sc.contains("position_y"))    s->positionY = sc["position_y"].get<float>();
        if (sc.contains("rotation"))      s->sensorRotationDeg = sc["rotation"].get<float>();
        if (sc.contains("mirror"))        s->mirrorAngles = sc["mirror"].get<bool>();
        if (sc.contains("which_meatbag")) s->whichMeatbag = sc["which_meatbag"].get<int>();
        if (sc.contains("show_info"))     s->showSensorInformation = sc["show_info"].get<bool>();

        if (dynamic_cast<OrbbecPulsar*>(s)) {
            if (sc.contains("motor_speed")) s->guiMotorSpeed = sc["motor_speed"].get<int>();
            if (sc.contains("fog_mode"))    s->guiSpecialWorkingMode = sc["fog_mode"].get<bool>();
        }
    }
}

void SensorManager::load(ofJson& config) {
    if (config.contains("number_sensors")) {
        int n = config["number_sensors"];
        loadSensors(n, config);
    }
    else {
        addSensor();
    }
}