#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofxGuiSetFont(ofToDataPath("Hack-Regular.ttf"), 11);
    ofxGuiSetBorderColor(ofColor::black);
    ofxGuiSetHeaderColor(ofColor::thistle);
    ofxGuiSetTextColor(ofColor::black);
    ofSetFrameRate(60);
    
    
    ofColor barColor = ofColor::snow;
    barColor.a = 210;
    ofxGuiSetBackgroundColor(barColor);
    barColor.a = 255;
    ofxGuiSetFillColor(barColor);
    ofxGuiEnableHiResDisplay();
    ofxGuiSetDefaultWidth(200);
    
    meatbagsGui.setup("general settings");
    meatbagsGui.setDefaultHeight(12);
    
    meatbagsSettings.setName("meatbags");
    meatbagsSettings.add(areaSize.set( "area size (m)", 5.0, 0.5, 20.0));
    meatbagsSettings.add(meatbags.epsilon.set( "cluster epsilon (mm)", 100, 1, 500));
    meatbagsSettings.add(meatbags.minPoints.set( "cluster min points", 10, 1, 150));
    meatbagsSettings.add(meatbags.blobPersistence.set("blob persistence (s)", 0.1, 0.0, 3.0));
    meatbagsSettings.add(headlessMode.set("headless mode (h)", false));
    meatbagsGui.add(meatbagsSettings);
    
    oscSettings.setName("OSC");
    oscSettings.add(oscSenderAddress.set( "OSC address", "192.168.0.11"));
    oscSettings.add(oscSenderPort.set( "OSC port", 5432, 4000, 12000));
    oscSettings.add(oscActive.set("OSC active", false));
    oscSettings.add(normalizeBlobs.set("normalize OSC output", false));
    meatbagsGui.add(oscSettings);
    
    sensorsSettings.setName("sensors");
    sensorsSettings.add(numberSensors.set("number sensors", 1, 1, 5));
    meatbagsGui.add(sensorsSettings);
 
    meatbagsGui.loadFromFile("generalSettings.json");
    meatbagsGui.maximize();
    
    space.width = ofGetWidth();
    space.height = ofGetHeight();
    space.areaSize = areaSize;
    space.origin = ofPoint(ofGetWidth() / 2.0, 50);
    setSpace();
    
    bounds = Bounds(4);
    boundsGui.setup("bounds");
    boundsGui.setDefaultHeight(12);
    boundsGui.setPosition(ofVec3f(ofGetWidth() - 210, 12));
    boundsGui.add(bounds.points[0].set("bounds p1", ofPoint(2.5, 2.5), ofPoint(-10.0, -10.0), ofPoint(10.0, 10.0)));
    boundsGui.add(bounds.points[1].set("bounds p2", ofPoint(2.5, 2.5), ofPoint(-10.0, -10.0), ofPoint(10.0, 10.0)));
    boundsGui.add(bounds.points[2].set("bounds p3", ofPoint(2.5, 2.5), ofPoint(-10.0, -10.0), ofPoint(10.0, 10.0)));
    boundsGui.add(bounds.points[3].set("bounds p4", ofPoint(2.5, 2.5), ofPoint(-10.0, -10.0), ofPoint(10.0, 10.0)));
    boundsGui.loadFromFile("boundsSettings.json");
    
    int n = numberSensors;
    setNumberSensors(n);
    
    for (int i = 0; i < sensorGuis.size(); i++) {
        string filename = "sensor" + to_string(i + 1) + "Settings.json";
        sensorGuis[i]->loadFromFile(filename);
    }
    
    numberSensors.addListener(this, &ofApp::setNumberSensors);
    areaSize.addListener(this, &ofApp::setAreaSize);
    oscSenderAddress.addListener(this, &ofApp::setOscSenderAddress);
    oscSenderPort.addListener(this, &ofApp::setOscSenderPort);
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

//--------------------------------------------------------------
void ofApp::update(){
    sensors.setBounds(bounds);
    sensors.update();
    meatbags.update();
    bounds.update();

    if (!sensors.areNewCoordinatesAvailable()) return;
    sensors.getCoordinatesAndIntensities(meatbags.coordinates, meatbags.intensities, meatbags.numberCoordinates);
    
    meatbags.updateBlobs();
    meatbags.getBlobs(blobs);
    
    if (oscActive) sendBlobOsc();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    if (headlessMode) return;
    
    viewer.drawGrid();
    viewer.drawBlobs(meatbags.oldBlobs);
    viewer.drawBounds(bounds);
    viewer.drawSensors(sensors);
    
    meatbagsGui.draw();
    boundsGui.draw();
    for (int i = sensorGuis.size() - 1; i >= 0; i--) {
        sensorGuis[i]->draw();
    }

    drawFps();
}

void ofApp::setNumberSensors(int & numberSensors) {
    if (numberSensors > sensors.hokuyos.size()) {
        while (sensors.hokuyos.size() < numberSensors) {
            addSensor();
        }
    }
    if (numberSensors < sensors.hokuyos.size()) {
        while (sensors.hokuyos.size() > numberSensors) {
            removeSensor();
        }
    }
}

void ofApp::addSensor() {
    int onesIndex = sensors.hokuyos.size() + 1;
    int currentIndex = sensors.hokuyos.size();

    ofColor randomColor = ofColor::fromHsb(ofRandom(0, 255),  255.0, 255.0);
    
    Hokuyo* hokuyo = new Hokuyo();
    ofxPanel * sensorGui =  NULL;
    sensorGui = new ofxPanel();
    sensorGui->setDefaultWidth(200 - 14);
    sensorGui->setup("sensor " + to_string(onesIndex) + " settings");
    sensorGui->add(hokuyo->sensorColor.set("color", randomColor));
    sensorGui->add(hokuyo->ipAddress.set("IP address", "0.0.0.0"));
    sensorGui->add(hokuyo->autoReconnectActive.set("auto reconnect", true));
    sensorGui->add(hokuyo->positionX.set("position x", currentIndex * 1.0, -10.0, 10.0));
    sensorGui->add(hokuyo->positionY.set("position y", 0.0, 0.0, 20.0));
    sensorGui->add(hokuyo->sensorRotationDeg.set( "sensor rotation (deg)", 0, -180.0, 180.0));
    sensorGui->add(hokuyo->mirrorAngles.set("mirror angles", false));
    sensorGui->add(hokuyo->showSensorInformation.set("show sensor info", false));
    hokuyo->setInfoPosition(10, ofGetHeight() - 10);
    sensors.addSensor(hokuyo);
    sensorGuis.push_back(sensorGui);
    float y = currentIndex * 123;
    sensorGuis[currentIndex]->setPosition(ofVec3f(14, 210 + y));
}

void ofApp::removeSensor() {
    int index = sensors.hokuyos.size() - 1;
    sensorGuis.pop_back();
    sensors.removeSensor();
}

void ofApp::drawFps() {
    std::stringstream strm;
    strm << setprecision(3) << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}

//--------------------------------------------------------------
void ofApp::exit(){
    meatbagsGui.saveToFile("generalSettings.json");
    boundsGui.saveToFile("boundsSettings.json");
    for (int i = 0; i < sensorGuis.size(); i++) {
        string filename = "sensor" + to_string(i + 1) + "Settings.json";
        sensorGuis[i]->saveToFile(filename);
    }
    sensors.closeSensors();
}

void ofApp::windowResized(int width, int height) {
    for (auto& sensor : sensors.hokuyos) {
        sensor->setInfoPosition(10, height - 10);
    }
    space.width = width;
    space.height = height;
    space.origin.x = width / 2.0;
    
    setSpace();
}

void ofApp::setSpace() {
    bounds.setSpace(space);
    viewer.setSpace(space);
    sensors.setSpace(space);
}

void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    areaSize -= scrollY * 0.15;
    areaSize = ofClamp(areaSize, areaSize.getMin(), areaSize.getMax());
}

void ofApp::setAreaSize(float &areaSize) {
    space.areaSize = areaSize;
    setSpace();
}

void ofApp::setOscSenderAddress(string& oscSenderAddress) {
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void ofApp::setOscSenderPort(int& oscSenderPort) {
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void ofApp::sendBlobOsc() {
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

void ofApp::keyPressed(int key) {
    if (key == 104) {
        headlessMode = !headlessMode;
    }
}
