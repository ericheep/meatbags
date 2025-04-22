#include "ofApp.h"
#define NUM_HOKUYOS 2

//--------------------------------------------------------------
void ofApp::setup(){
    ofxGuiSetFont(ofToDataPath("Hack-Regular.ttf"), 11);
    ofxGuiSetBorderColor(ofColor::black);
    ofxGuiSetHeaderColor(ofColor::thistle);
    ofxGuiSetTextColor(ofColor::black);
    
    ofColor barColor = ofColor::snow;
    barColor.a = 210;
    ofxGuiSetBackgroundColor(barColor);
    barColor.a = 255;
    ofxGuiSetFillColor(barColor);
    ofxGuiEnableHiResDisplay();
    ofxGuiSetDefaultWidth(200);

    gui.setup("meatbags");
    gui.setDefaultHeight(12);
    
    meatbagsSettings.setName("meatbags settings");
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
        Hokuyo* hokuyo = new Hokuyo();
        
        ofParameterGroup sensorSetting;
        sensorSettings.push_back(sensorSetting);
        sensorSettings[i].setName("sensor " + to_string(i + 1) + " settings");
        sensorSettings[i].add(hokuyo->sensorColor.set("color", ofColor::lightSeaGreen));
        sensorSettings[i].add(hokuyo->ipAddress.set("IP address", "192.168.0.10"));
        sensorSettings[i].add(hokuyo->mirrorAngles.set("mirror angles", false));
        sensorSettings[i].add(hokuyo->positionX.set("position x", 0.0, -10.0, 10.0));
        sensorSettings[i].add(hokuyo->positionY.set("position y", 0.0, 0.0, 20.0));
        sensorSettings[i].add(hokuyo->autoReconnectActive.set("auto reconnect", true));
        sensorSettings[i].add(hokuyo->sensorRotationDeg.set( "sensor rotation (deg)", 0, -180.0, 180.0));
        sensorSettings[i].add(hokuyo->showSensorInformation.set("show sensor info", true));
                
        sensors.addSensor(hokuyo);
        gui.add(sensorSettings[i]);
    }
    
    gui.loadFromFile("settings.xml");
    gui.maximize();
    
    bounds.setCanvasSize(ofGetWidth(), ofGetHeight());
    bounds.setAreaSize(areaSize);
    bounds.setBounds(boundsX1, boundsX2, boundsY1, boundsY2);
    
    viewer.setSensorColors(sensorColors);
    viewer.setCanvasSize(ofGetWidth(), ofGetHeight());
    viewer.setAreaSize(areaSize);
    viewer.setBounds(bounds);
    
    meatbags.setEpsilon(epsilon);
    meatbags.setMinPoints(minPoints);
    meatbags.setBlobPersistence(blobPersistence);

    areaSize.addListener(this, &ofApp::setAreaSize);
    boundsX1.addListener(this, &ofApp::setBoundsX1);
    boundsX2.addListener(this, &ofApp::setBoundsX2);
    boundsY1.addListener(this, &ofApp::setBoundsY1);
    boundsY2.addListener(this, &ofApp::setBoundsY2);
    
    epsilon.addListener(this, &ofApp::setEpsilon);
    minPoints.addListener(this, &ofApp::setMinPoints);
    blobPersistence.addListener(this, &ofApp::setBlobPersistence);
    
    oscSenderAddress.addListener(this, &ofApp::setOscSenderAddress);
    oscSenderPort.addListener(this, &ofApp::setOscSenderPort);
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

//--------------------------------------------------------------
void ofApp::update(){
    sensors.setBounds(bounds);
    viewer.setBounds(bounds);
    sensors.update();
    meatbags.update();
    updateGuiBounds();

    if (!sensors.areNewCoordinatesAvailable()) return;
    sensors.getCoordinatesAndIntensities(meatbags.coordinates, meatbags.intensities, meatbags.numberCoordinates);
    
    meatbags.updateBlobs();
    meatbags.getBlobs(blobs);
    
    if (oscActive) sendBlobOsc();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    viewer.drawGrid();
    viewer.drawBlobs(meatbags.oldBlobs);
    viewer.drawBounds();
    viewer.drawDraggablePoints();
    
    for (auto& sensor : sensors.hokuyos) {
        viewer.drawCoordinates(sensor->coordinates, sensor->sensorColor);
        viewer.drawSensor(sensor->position, sensor->sensorRotationRad, sensor->sensorColor);
        if (sensor->showSensorInformation) sensor->draw();
    }

    gui.draw();
    drawFps();
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

void ofApp::drawFps() {
    std::stringstream strm;
    strm << setprecision(3) << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}

//--------------------------------------------------------------
void ofApp::exit(){
    gui.saveToFile("settings.xml");
    sensors.closeSensors();
}

void ofApp::windowResized(int width, int height) {
    for (auto& sensor : sensors.hokuyos) {
        sensor->setInfoPosition(10, height - 10);
    }
    viewer.setCanvasSize(width, height);
    bounds.setCanvasSize(width, height);
}

void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    areaSize -= scrollY * 0.15;
    areaSize = ofClamp(areaSize, areaSize.getMin(), areaSize.getMax());
}

void ofApp::setAreaSize(float &areaSize) {
    viewer.setAreaSize(areaSize);
    bounds.setAreaSize(areaSize);
    bounds.setBounds(boundsX1, boundsX2, boundsY1, boundsY2);
}

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
