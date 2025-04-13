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
    gui.add(areaY2.set( "area y2", 5.0, 0, 5.0));
   
    gui.add(boundsX1.set( "bounds x1", -1.0, -2.5, 0.0));
    gui.add(boundsX2.set( "bounds x2", 1.0, 0, 2.5));
    gui.add(boundsY1.set( "bounds y1", 0.0, 0, 5.0));
    gui.add(boundsY2.set( "bounds y2", 2.5, 0, 5.0));
    
    gui.add(epsilon.set( "epsilon", 100, 1, 500));
    gui.add(minPoints.set( "min points", 10, 1, 50));
        
    meatbags.setSize(900, 900);
    meatbags.setScanningArea(areaX1, areaX2, areaY1, areaY2);
    meatbags.setFilterBounds(boundsX1, boundsX2, boundsY1, boundsY2);
    meatbags.setEpsilon(epsilon);
    
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
