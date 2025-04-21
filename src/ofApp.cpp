#include "ofApp.h"
#define NUM_HOKUYOS 3

//--------------------------------------------------------------
void ofApp::setup(){
    ofxGuiSetFont(ofToDataPath("Hack-Regular.ttf"), 11);
    gui.setup("meatbags");
    gui.setDefaultHeight(12);
    
    meatbagsSettings.setName("general settings");
    meatbagsSettings.add(areaSize.set( "area size (m)", 5.0, 0.5, 20.0));
    meatbagsSettings.add(epsilon.set( "cluster epsilon (mm)", 100, 1, 500));
    meatbagsSettings.add(minPoints.set( "cluster min points", 10, 1, 50));
    meatbagsSettings.add(blobPersistence.set("blob persistence (s)", 0.1, 0.0, 3.0));
    meatbagsSettings.add(boundsX1.set("bounds x1", -2.5, -10.0, 0.0));
    meatbagsSettings.add(boundsX2.set("bounds x2", 2.5, 0.0, 10.0));
    meatbagsSettings.add(boundsY1.set("bounds y1", 1.0, 0.0, 20.0));
    meatbagsSettings.add(boundsY2.set("bounds y2", 5.0, 0.0, 20.0));
    gui.add(meatbagsSettings);
    
    oscSettings.setName("OSC settings");
    oscSettings.add(oscSenderAddress.set( "OSC address", "192.168.0.11"));
    oscSettings.add(oscSenderPort.set( "OSC port", 5432, 4000, 12000));
    oscSettings.add(oscActive.set("OSC active", false));
    oscSettings.add(normalizeBlobs.set("normalize OSC output", false));
    gui.add(oscSettings);
    
    for (int i = 0; i < NUM_HOKUYOS; i++) {
        ofParameterGroup sensorSetting;
        sensorSettings.push_back(sensorSetting);
        
        ofParameter<string> sensorIPAddress;
        ofParameter<bool> mirrorAngle;
        ofParameter<float> positionX;
        ofParameter<float> positionY;
        ofParameter<bool> autoReconnectActive;
        ofParameter<float> sensorRotation;
        ofParameter<bool> showSensorInformation;
        
        sensorIPAddresses.push_back(sensorIPAddress);
        autoReconnectsActive.push_back(autoReconnectActive);
        positionXs.push_back(positionX);
        positionYs.push_back(positionY);
        mirrorAngles.push_back(mirrorAngle);
        sensorRotations.push_back(sensorRotation);
        showSensorInformations.push_back(showSensorInformation);
        
        sensorSettings[i].setName("sensor " + to_string(i) + " settings");
        sensorSettings[i].add(sensorIPAddress.set("IP address", "192.168.0.10"));
        sensorSettings[i].add(autoReconnectActive.set("auto reconnect", true));
        sensorSettings[i].add(positionX.set("position x", 0.0, -10.0, 10.0));
        sensorSettings[i].add(positionY.set("position y", 0.0, 0.0, 20.0));
        sensorSettings[i].add(mirrorAngle.set("mirror angles", false));
        sensorSettings[i].add(sensorRotation.set( "sensor rotation (deg)", 0, -180.0, 180.0));
        sensorSettings[i].add(showSensorInformation.set("show sensor info", true));
        gui.add(sensorSettings[i]);
    }
    
    gui.loadFromFile("settings.xml");
    gui.maximize();
    
    for (int i = 0; i < NUM_HOKUYOS; i++) {
        Hokuyo* hokuyo = new Hokuyo();
        hokuyo->setup(sensorIPAddresses[i], 10940);
        hokuyo->setAutoReconnect(autoReconnectsActive[i]);
        hokuyo->setPosition(positionXs[i], positionYs[i]);
        hokuyo->setMirrorAngles(mirrorAngles[i]);
        
        SensorParameterFunction* sensorFunctions;
        sensorFunctions = new SensorParameterFunction(hokuyo);
        
        sensorIPAddresses[i].addListener(this, sensorFunctions->setIPAddress);
        
        hokuyos.push_back(hokuyo);
    }
    
    bounds.setCanvasSize(ofGetWidth(), ofGetHeight());
    bounds.setAreaSize(areaSize);
    bounds.setBounds(boundsX1, boundsX2, boundsY1, boundsY2);
    
    viewer.setCanvasSize(ofGetWidth(), ofGetHeight());
    viewer.setAreaSize(areaSize);
    viewer.setBounds(bounds);
    
    meatbags.setEpsilon(epsilon);
    meatbags.setMinPoints(minPoints);
    meatbags.setBlobPersistence(blobPersistence);
    meatbags.setBounds(bounds);
    
    boundsX1.addListener(this, &ofApp::setBoundsX1);
    boundsX2.addListener(this, &ofApp::setBoundsX2);
    boundsY1.addListener(this, &ofApp::setBoundsY1);
    boundsY2.addListener(this, &ofApp::setBoundsY2);
    
    /*sensorIPAddress.addListener(this, &ofApp::setIPAddress);
    positionX.addListener(this, &ofApp::setPositionX);
    positionY.addListener(this, &ofApp::setPositionY);
  
    sensorRotation.addListener(this, &ofApp::setSensorRotation);
    blobPersistence.addListener(this, &ofApp::setBlobPersistence);
    autoReconnectActive.addListener(this, &ofApp::setAutoReconnect);
    areaSize.addListener(this, &ofApp::setAreaSize);
    mirrorAngles.addListener(this, &ofApp::setMirrorAngles);
     */
    
    epsilon.addListener(this, &ofApp::setEpsilon);
    minPoints.addListener(this, &ofApp::setMinPoints);

    oscSenderAddress.addListener(this, &ofApp::setOscSenderAddress);
    oscSenderPort.addListener(this, &ofApp::setOscSenderPort);
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

//--------------------------------------------------------------
void ofApp::update(){
    for (auto& hokuyo : hokuyos) {
        hokuyo->update();
    }
    
    meatbags.update();
    viewer.setBounds(bounds);
    meatbags.setBounds(bounds);
    updateGuiBounds();
    
    for (auto& hokuyo : hokuyos) {
        if (!hokuyo->newCoordinatesAvailable) return;
        hokuyo->newCoordinatesAvailable = false;
        
        hokuyo->getCoordinates(meatbags.coordinates);
        hokuyo->getIntensities(meatbags.intensities);
    }
        
    meatbags.updateBlobs();
    meatbags.getBlobs(blobs);
    sendBlobOsc();
}

void ofApp::updateGuiBounds() {
    if (boundsX1 != bounds.x1) {
        boundsX1 = bounds.x1;
    }
    
    if (boundsX2 != bounds.x2) {
        boundsX2 = bounds.x2;
    }
    
    if (boundsY1 != bounds.y1) {
        boundsY1 = bounds.y1;
    }
    
    if (boundsY2 != bounds.y2) {
        boundsY2 = bounds.y2;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    viewer.drawGrid();
    viewer.drawBlobs(meatbags.oldBlobs);
    viewer.drawBounds();
    viewer.drawDraggablePoints();
    
    for (auto& hokuyo : hokuyos) {
        viewer.drawCoordinates(hokuyo->coordinates, ofColor::greenYellow);
        viewer.drawSensor(hokuyo->position, hokuyo->sensorRotation, ofColor::greenYellow);
    }
    
    for (int i = 0; i < NUM_HOKUYOS; i++) {
        // if (showSensorInformations[i]) hokuyos[i].draw();
    }
    
    gui.draw();
    drawFps();
}

void ofApp::drawFps() {
    std::stringstream strm;
    strm << setprecision(3) << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}

//--------------------------------------------------------------
void ofApp::exit(){
    gui.saveToFile("settings.xml");
    for (auto& hokuyo : hokuyos) {
        hokuyo->close();
    }
}

void ofApp::windowResized(int width, int height) {
    for (auto& hokuyo : hokuyos) {
        hokuyo->setInfoPosition(10, height - 10);
    }
    viewer.setCanvasSize(width, height);
    bounds.setCanvasSize(width, height);
}

void ofApp::setAreaSize(float &areaSize) {
    viewer.setAreaSize(areaSize);
    bounds.setAreaSize(areaSize);
    bounds.setBounds(boundsX1, boundsX2, boundsY1, boundsY2);
}

/*void ofApp::setIPAddress(string &ipAddress) {
    hokuyos[0]->setIPAddress(ipAddress);
}

 void ofApp::setAutoReconnect(bool &autoReconnectActive) {
     hokuyo.setAutoReconnect(autoReconnectActive);
 }

 void ofApp::setMirrorAngles(bool &mirrorAngles) {
     hokuyo.setMirrorAngles(mirrorAngles);
 }
 
void ofApp::setPositionX(float &x) {
    hokuyo.setPosition(x, positionY);
}

void ofApp::setPositionY(float &y) {
    hokuyo.setPosition(positionX, y);
}
*/

void ofApp::setBoundsX1(float &x1) {
    bounds.setBounds(x1, boundsX2, boundsY1, boundsY2);
}

void ofApp::setBoundsX2(float &x2) {
    bounds.setBounds(boundsX1, x2, boundsY1, boundsY2);
}

void ofApp::setBoundsY1(float &y1) {
    bounds.setBounds(boundsX1, boundsX2, y1, boundsY2);
}

void ofApp::setBoundsY2(float &y2) {
    bounds.setBounds(boundsX1, boundsX2, boundsY1, y2);
}

/*
void ofApp::setSensorRotation(float &sensorRotation) {
    hokuyo.setSensorRotation(sensorRotation);
}

void ofApp::setBlobPersistence(float &blobPersistence) {
    meatbags.setBlobPersistence(blobPersistence);
*/


void ofApp::setEpsilon(float &epsilon) {
    meatbags.setEpsilon(epsilon);
}

void ofApp::setMinPoints(int &minPoints) {
    meatbags.setMinPoints(minPoints);
}

void ofApp::setOscSenderAddress(string& oscSenderAddress) {
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void ofApp::setOscSenderPort(int& oscSenderPort) {
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void ofApp::sendBlobOsc() {
    if (!oscActive) return;
        
    for (auto& blob : blobs) {
        ofxOscMessage msg;
        msg.setAddress("/blobs");
        msg.addIntArg(blob.index);
        
        // millimeters to meters
        float x = blob.centroid.x * 0.001;
        float y = blob.centroid.y * 0.001;
        
        if (normalizeBlobs) {
            x = ofMap(x, meatbags.boundsX1, meatbags.boundsX2, 0.0, 1.0);
            y = ofMap(y, meatbags.boundsY1, meatbags.boundsY2, 0.0, 1.0);
        }
        
        msg.addFloatArg(x);
        msg.addFloatArg(y);
        msg.addFloatArg(blob.intensity);
        msg.addFloatArg(blob.distanceFromSensor);
        
        oscSender.sendMessage(msg);
    }
}
