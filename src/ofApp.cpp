#include "ofApp.h"
#define PORT 10940
#define IP "192.168.0.10"

//--------------------------------------------------------------
void ofApp::setup(){
    globalFont.setGlobalDpi(72);
    globalFont.load("Hack-Bold.ttf", 14);
    meatbags.setFont(globalFont);
    globalFont.load("Hack-Regular.ttf", 11);
    hokuyo.setFont(globalFont);
    
    ofxGuiSetFont("Hack-Regular.ttf", 11);
    gui.setup("meatbags");
    gui.setDefaultHeight(12);
    
    meatbagsSettings.setName("general settings");
    meatbagsSettings.add(areaSize.set( "area size (m)", 5.0, 0.5, 20.0));
    meatbagsSettings.add(mirrorX.set("mirror x", false));
    meatbagsSettings.add(epsilon.set( "cluster epsilon (mm)", 100, 1, 500));
    meatbagsSettings.add(minPoints.set( "cluster min points", 10, 1, 50));
    meatbagsSettings.add(blobPersistence.set("blob persistence (s)", 0.1, 0.0, 3.0));
    gui.add(meatbagsSettings);
    
    oscSettings.setName("OSC settings");
    oscSettings.add(oscSenderAddress.set( "OSC address", "192.168.0.11"));
    oscSettings.add(oscSenderPort.set( "OSC port", 5432, 4000, 12000));
    oscSettings.add(oscActive.set("OSC active", false));
    oscSettings.add(normalizeBlobs.set("normalize OSC output", false));
    gui.add(oscSettings);
    
    sensorSettings.setName("sensor settings");
    sensorSettings.add(sensorRotation.set( "sensor rotation (rad)", 0, -PI, PI));
    sensorSettings.add(autoReconnectActive.set("auto reconnect", true));
    sensorSettings.add(showSensorInformation.set("show sensor info", true));
    gui.add(sensorSettings);
    
    gui.loadFromFile("settings.xml");
    gui.maximize();

    hokuyo.setup(IP, PORT);
    hokuyo.setSensorRotation(sensorRotation);
    hokuyo.setRectangle(10, ofGetHeight() - 350, ofGetWidth() / 2.0, 340);
    hokuyo.setAutoReconnect(autoReconnectActive);
    
    meatbags.setMirrorX(mirrorX);
    meatbags.setCanvasSize(ofGetWidth(), ofGetHeight());
    meatbags.setAreaSize(areaSize);
    meatbags.setBlobPersistence(blobPersistence);
    meatbags.setEpsilon(epsilon);
    meatbags.setMinPoints(minPoints);
    
    sensorRotation.addListener(this, &ofApp::setSensorRotation);
    sensorMotorSpeed.addListener(this, &ofApp::setSensorMotorSpeed);
    blobPersistence.addListener(this, &ofApp::setBlobPersistence);
    autoReconnectActive.addListener(this, &ofApp::setAutoReconnect);
    areaSize.addListener(this, &ofApp::setAreaSize);
    mirrorX.addListener(this, &ofApp::setMirrorX);
    epsilon.addListener(this, &ofApp::setEpsilon);
    minPoints.addListener(this, &ofApp::setMinPoints);
    oscSenderAddress.addListener(this, &ofApp::setOscSenderAddress);
    oscSenderPort.addListener(this, &ofApp::setOscSenderPort);
    
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

//--------------------------------------------------------------
void ofApp::update(){
    hokuyo.update();
    meatbags.update();
    
    if (!hokuyo.newCoordinatesAvailable) return;
        
    hokuyo.getPolarCoordinates(meatbags.polarCoordinates);
    hokuyo.getIntensities(meatbags.intensities);
        
    meatbags.updateBlobs();
    meatbags.getBlobs(blobs);
    sendBlobOsc();
    
    hokuyo.newCoordinatesAvailable = false;
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    meatbags.draw();
    if (showSensorInformation) hokuyo.draw();
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
    meatbags.saveToFile("meatbags.xml");
    hokuyo.close();
}

void ofApp::windowResized(int width, int height) {
    hokuyo.setRectangle(10, height - 350, width / 2.0, 340);
    meatbags.setCanvasSize(width, height);
}

void ofApp::setAutoReconnect(bool &autoReconnectActive) {
    hokuyo.setAutoReconnect(autoReconnectActive);
}

void ofApp::setSensorMotorSpeed(int &sensorMotorSpeed) {
    hokuyo.sendSetMotorSpeedCommand(sensorMotorSpeed);
}

void ofApp::setMirrorX(bool &mirrorX) {
    meatbags.setMirrorX(mirrorX);
}

void ofApp::setAreaSize(float &areaSize) {
    meatbags.setAreaSize(areaSize);
}

void ofApp::setSensorRotation(float &sensorRotation) {
    hokuyo.setSensorRotation(sensorRotation);
}

void ofApp::setBlobPersistence(float &blobPersistence) {
    meatbags.setBlobPersistence(blobPersistence);
}

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
