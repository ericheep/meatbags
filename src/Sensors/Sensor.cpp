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
    
    statusTimer = 0.0;
    statusTimeInterval = 0.2;
    threadInactiveTimer = 0.0;
    threadInactiveTimeInterval = 3.0;
    reconnectionTimer = 0.0;
    reconnectionTimeInterval = 3.0;
    
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
    
    sensorTypes.add("Hokuyo");
    sensorTypes.add("Orbbec Pulsar");
    sensorTypes.disableMultipleSelection();
}

Sensor::~Sensor() {
    ipAddress.removeListener(this, &Sensor::setIPAddress);
    positionX.removeListener(this, &Sensor::setPositionX);
    positionY.removeListener(this, &Sensor::setPositionY);
    mirrorAngles.removeListener(this, &Sensor::setMirrorAngles);
    sensorRotationDeg.removeListener(this, &Sensor::setSensorRotation);
}

void Sensor::close() {
    if (isThreadRunning()) {
        stopThread();
        sleep(100);
    }
    
    tcpClient.close();
}

void Sensor::shutdown() {
    showSensorInformation = false;
    ipAddress.removeListener(this, &Sensor::setIPAddress);
    positionX.removeListener(this, &Sensor::setPositionX);
    positionY.removeListener(this, &Sensor::setPositionY);
    mirrorAngles.removeListener(this, &Sensor::setMirrorAngles);
    sensorRotationDeg.removeListener(this, &Sensor::setSensorRotation);
    isConnected = false;
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

void Sensor::drawSensorInfo(vector<string> sensorInfoLines) {
    float textBoxHeight = sensorInfoLines.size() * 16;
    float textBoxWidth = 400;
    
    float textX = x - textBoxWidth * 0.5;
    float textY = y - textBoxHeight * 0.5;
    
    ofFill();
    ofSetColor(0, 0, 0, 200);
    ofRectangle textBox;
    textBox.setFromCenter(x, y, textBoxWidth, textBoxHeight + 8);
    ofDrawRectangle(textBox);
    ofSetColor(sensorColor);
    ofNoFill();
    ofDrawRectangle(textBox);
    ofFill();
    
    for (int i = 0; i < sensorInfoLines.size(); i++) {
        float yPos = textY + 16 * i;
        ofDrawBitmapString(sensorInfoLines[i], textX + 6, yPos + 10);
    }
}

void Sensor::setIPAddress(string &ipAddress) {
    // will be overridden
}

void Sensor::setSpace(Space& _space) {
    space = _space;
    scale = space.width / (space.areaSize * 1000.0);
}

void Sensor::setTranslation(ofPoint _translation) {
    translation = _translation;
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
ofPoint Sensor::convertCoordinateToScreenPoint(ofPoint coordinate) {
    return coordinate * scale + space.origin + translation;
}

ofPoint Sensor::convertScreenPointToCoordinate(ofPoint screenPoint) {
    return (screenPoint - space.origin - translation) / scale * 0.001;
}

bool Sensor::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    ofPoint screenPoint = convertCoordinateToScreenPoint(position);
    float distance = mousePoint.distance(screenPoint);
    
    if(distance <= position.halfSize) {
        position.isMouseOver = true;
    } else {
        position.isMouseOver = false;
    }
    
    float noseX = cos(sensorRotationRad - HALF_PI);
    float noseY = sin(sensorRotationRad - HALF_PI);
    
    ofPoint offsetPoint = ofPoint(noseX, noseY) * noseRadius;
    ofPoint nosePoint = screenPoint - offsetPoint;
    
    if (mousePoint.distance(nosePoint) < nosePosition.halfSize) {
        nosePosition.isMouseOver = true;
    } else {
        nosePosition.isMouseOver = false;
    }
    
    return false;
}

bool Sensor::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    if (position.isMouseOver) {
        position.isMouseClicked = true;
    } else {
        position.isMouseClicked = false;
    }
    
    if (nosePosition.isMouseOver) {
        nosePosition.isMouseClicked = true;
    } else {
        nosePosition.isMouseClicked = false;
    }
    
    return false;
}

bool Sensor::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    if (position.isMouseClicked) {
        ofPoint coordinate = convertScreenPointToCoordinate(mousePoint);
        positionX = coordinate.x;
        positionY = coordinate.y;
        return true;
    }
    
    if (nosePosition.isMouseClicked) {
        ofPoint screenPoint = convertCoordinateToScreenPoint(position);
        float angle = atan2(screenPoint.y - mousePoint.y, screenPoint.x - mousePoint.x);
        sensorRotationDeg = (angle + HALF_PI) * 180.0 / PI;
        
        return true;
    }
    
    return false;
}

bool Sensor::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    if (position.isMouseClicked) {
        position.isMouseClicked = false;
    }
    
    return false;
}
