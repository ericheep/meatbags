#include "ofApp.h"
#define PORT 10940
#define IP "192.168.0.10"

//--------------------------------------------------------------
void ofApp::setup(){
    hokuyo.setup(IP, PORT);

    ofBackground(200);
       
    gui.setup("ofx-Hokuyo");
    gui.add(scale.set( "scale", 0.5, 0.1, 1.0));
    
    gui.add(areaX1.set( "area x1", -2.5, -5.0, 0.0));
    gui.add(areaX2.set( "area x2", 2.5, 0, 5.0));
    gui.add(areaY1.set( "area y1", 0.0, 0, 5.0));
    gui.add(areaY2.set( "area y2", 5.0, 0, 10.0));
   
    gui.add(boundsX1.set( "bounds x1", -1.0, -2.5, 0.0));
    gui.add(boundsX2.set( "bounds x2", 1.0, 0, 2.5));
    gui.add(boundsY1.set( "bounds y1", 0.0, 0, 5.0));
    gui.add(boundsY2.set( "bounds y2", 2.5, 0, 10.0));
    
    gui.add(epsilon.set( "cluster epsilon", 100, 1, 500));
    gui.add(minPoints.set( "cluster min points", 10, 1, 50));
    
    gui.add(normalizeBlobs.set("normalize blobs", true));
    gui.add(oscSenderAddress.set( "OSC address", "192.168.0.11"));
    gui.add(oscSenderPort.set( "OSC port", 5432, 4000, 12000));
        
    meatbags.setSize(ofGetWidth() - 100, ofGetHeight() - 100);
    meatbags.setScanningArea(areaX1, areaX2, areaY1, areaY2);
    meatbags.setFilterBounds(boundsX1, boundsX2, boundsY1, boundsY2);
    meatbags.setEpsilon(epsilon);
    
    oscSenderAddress.addListener(this, &ofApp::setOscSenderAddress);
    oscSenderPort.addListener(this, &ofApp::setOscSenderPort);

    oscSender.setup(oscSenderAddress, oscSenderPort);
    
    gui.loadFromFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::update(){
    hokuyo.update();
    
    hokuyo.getPolarCoordinates(meatbags.polarCoordinates);
    hokuyo.getIntensities(meatbags.intensities);
    
    meatbags.setScale(scale);
    meatbags.setScanningArea(areaX1, areaX2, areaY1, areaY2);
    meatbags.setFilterBounds(boundsX1, boundsX2, boundsY1, boundsY2);
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
    
    ofPushMatrix();
    ofTranslate(50, 50);
    meatbags.draw();
    ofPopMatrix();
    
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
    hokuyo.close();
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
            x = ofMap(x, boundsX1, boundsX2, 0.0, 1.0);
            y = ofMap(y, boundsY1, boundsY2, 0.0, 1.0);
        }
        
        msg.addFloatArg(x);
        msg.addFloatArg(y);
        
        oscSender.sendMessage(msg);
    }
}
