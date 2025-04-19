#include "ofApp.h"
#define PORT 10940
#define IP "192.168.0.10"

//--------------------------------------------------------------
void ofApp::setup(){
    hokuyo.setup(IP, PORT);
       
    gui.setup("ofx-Hokuyo");
    gui.setDefaultHeight(13);

    gui.add(areaSize.set( "area size", 0.5, 0.5, 20.0));
    gui.add(epsilon.set( "cluster epsilon", 100, 1, 500));
    gui.add(minPoints.set( "cluster min points", 10, 1, 50));
    
    gui.add(oscSenderAddress.set( "OSC address", "192.168.0.11"));
    gui.add(oscSenderPort.set( "OSC port", 5432, 4000, 12000));
    gui.add(normalizeBlobs.set("normalize OSC output", false));
    gui.add(oscEnabled.set("OSC enabled", false));
    gui.loadFromFile("settings.xml");

    meatbags.setCanvasSize(ofGetWidth(), ofGetHeight());
    meatbags.setAreaSize(areaSize);
    meatbags.setEpsilon(epsilon);
    
    oscSenderAddress.addListener(this, &ofApp::setOscSenderAddress);
    oscSenderPort.addListener(this, &ofApp::setOscSenderPort);
    oscSender.setup(oscSenderAddress, oscSenderPort);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    hokuyo.update();
    
    hokuyo.getPolarCoordinates(meatbags.polarCoordinates);
    hokuyo.getIntensities(meatbags.intensities);
    meatbags.setAreaSize(areaSize);
    
    meatbags.setEpsilon(epsilon);
    meatbags.setMinPoints(minPoints);
    meatbags.update();
    
    // get blobs to send out via OSC
    meatbags.getBlobs(blobs);
    sendBlobOsc();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    meatbags.draw();
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

void ofApp::setOscSenderAddress(string& oscSenderAddress) {
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void ofApp::setOscSenderPort(int& oscSenderPort) {
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void ofApp::sendBlobOsc() {
    if (!oscEnabled) return;
        
    for (auto& blob : blobs) {
        ofxOscMessage msg;
        msg.setAddress("/blobs");
        msg.addIntArg(blob.index);
        
        // millimeters to meters
        float x = blob.centroid.x * 0.001;
        float y = blob.centroid.y * 0.001;
        
        if (normalizeBlobs) {
            // x = ofMap(x, boundsX1, boundsX2, 0.0, 1.0);
            // y = ofMap(y, boundsY1, boundsY2, 0.0, 1.0);
        }
        
        msg.addFloatArg(x);
        msg.addFloatArg(y);
        msg.addFloatArg(blob.intensity);
        msg.addFloatArg(blob.distanceFromSensor);
        
        oscSender.sendMessage(msg);
    }
}
