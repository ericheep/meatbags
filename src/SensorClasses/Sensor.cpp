//
//  Sensor.cpp
//  meatbags

#include "Sensor.hpp"

Sensor::Sensor() {
    isConnected = false;

    autoReconnectActive = true;
    newCoordinatesAvailable = false;
    
    lastFrameTime = 0.0;
    sensorRotationDeg = 0;
    sensorRotationRad = 0;
    
    position = DraggablePoint();
    position.size = 15;
    position.halfSize = position.size * 0.5;
    nosePosition.size = 12;
    nosePosition.halfSize = nosePosition.size * 0.5;
    noseRadius = position.size + position.halfSize;
    
    position.x = 0.0;
    position.y = 0.0;
    position.isMouseOver = false;
    position.isMouseClicked = false;
}

Sensor::~Sensor() {
    ipAddress.removeListener(this, &Sensor::setIPAddress);
    positionX.removeListener(this, &Sensor::setPositionX);
    positionY.removeListener(this, &Sensor::setPositionY);
    mirrorAngles.removeListener(this, &Sensor::setMirrorAngles);
    sensorRotationDeg.removeListener(this, &Sensor::setSensorRotation);
}

void Sensor::setupParameters() {
    ipAddress.addListener(this, &Sensor::setIPAddress);
    positionX.addListener(this, &Sensor::setPositionX);
    positionY.addListener(this, &Sensor::setPositionY);
    mirrorAngles.addListener(this, &Sensor::setMirrorAngles);
    sensorRotationDeg.addListener(this, &Sensor::setSensorRotation);
    mirrorAngles = false;
}

void Sensor::initializeVectors() {
    angles.clear();
    coordinates.clear();
    intensities.clear();
    
    angles.resize(angularResolution);
    coordinates.resize(angularResolution);
    intensities.resize(angularResolution);
    
    bool m = mirrorAngles;
    setMirrorAngles(m);
}

void Sensor::setLocalIPAddress(string & _localIPAddress) {
    localIPAddress = _localIPAddress;
}

void Sensor::update() {
}

void Sensor::setIPAddress(string &ipAddress) {
    // will be overridden
}

void Sensor::setSensorRotation(float& _sensorRotationDeg) {
    sensorRotationRad = _sensorRotationDeg / 360.0 * TWO_PI;
}

void Sensor::setInterfaceAndIP(string _interface, string _localIP) {
    interface = _interface;
    localIPAddress = _localIP;
}

void Sensor::setInfoPosition(float _x, float _y) {
    x = _x;
    y = _y;
}

void Sensor::setPositionX(float &positionX) {
    position.x = positionX * 1000.0;
}

void Sensor::setPositionY(float &positionY) {
    position.y = positionY * 1000.0;
}

void Sensor::createCoordinate(int index, float distance) {
    float theta = angles[index] + sensorRotationRad;
    
    float x = cos(theta) * distance;
    float y = sin(theta) * distance;
    
    coordinates[index].set(ofPoint(x, y) + ofPoint(position.x, position.y));
}

void Sensor::setMirrorAngles(bool &_mirrorAngles) {
    mirrorAngles = _mirrorAngles;
    
    // -45 degrees offset
    float thetaOffset = -HALF_PI * 0.5;
    
    for (int i = 0; i < angularResolution; i++) {
        int index = i;
        float theta = (float) index / angularResolution * TWO_PI;
        
        if (mirrorAngles) {
            index = angularResolution - 1;
            thetaOffset = -HALF_PI * 0.5 - HALF_PI;
            theta *= -1.0;
        }

        angles[i] = theta + thetaOffset;
    }
}
