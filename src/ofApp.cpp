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
    meatbagsSettings.add(areaSize.set( "area size (m)", 10.0, 0.5, 30.0));
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
    sensorsSettings.add(numberSensors.set("number sensors", 1, 1, 8));
    meatbagsGui.add(sensorsSettings);
 
    meatbagsGui.loadFromFile("generalSettings.json");
    meatbagsGui.maximize();
    
    filtersGui.setup("filters");
    filtersGui.setPosition(ofGetWidth() - 210, 15);
    filtersSettings.setName("filters");
    // filtersSettings.add(addFilterButton.setup("add filter"));
    // filtersSettings.add(numberFilters.set("number filters", 1, 1, 30));

    filtersSettings.add(numberFilters.set("number filters", 1, 1, 30));
    filtersGui.add(filtersSettings);
    
    filtersGui.loadFromFile("filtersSettings.json");
    
    int n = numberSensors;
    setNumberSensors(n);
    
    for (int i = 0; i < sensorGuis.size(); i++) {
        string filename = "sensor" + to_string(i + 1) + "Settings.json";
        sensorGuis[i]->loadFromFile(filename);
    }
    
    n = numberFilters;
    setNumberFilters(n);
    
    for (int i = 0; i < filterGuis.size(); i++) {
        string filename = "filter" + to_string(i + 1) + "Settings.json";
        filterGuis[i]->loadFromFile(filename);
    }
    
    numberSensors.addListener(this, &ofApp::setNumberSensors);
    numberFilters.addListener(this, &ofApp::setNumberFilters);
    areaSize.addListener(this, &ofApp::setAreaSize);
    oscSenderAddress.addListener(this, &ofApp::setOscSenderAddress);
    oscSenderPort.addListener(this, &ofApp::setOscSenderPort);
    oscSender.setup(oscSenderAddress, oscSenderPort);
    
    space.width = ofGetWidth();
    space.height = ofGetHeight();
    space.areaSize = areaSize;
    space.origin = ofPoint(ofGetWidth() / 2.0, 50);
    setSpace();
}

//--------------------------------------------------------------
void ofApp::update(){
    sensors.setFilters(filters);
    sensors.update();
    meatbags.update();
    filters.update();

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
    viewer.drawFilters(filters);
    viewer.drawSensors(sensors, filters);
    
    meatbagsGui.draw();
    filtersGui.draw();
    
    for (int i = sensorGuis.size() - 1; i >= 0; i--) {
        sensorGuis[i]->draw();
    }
    
    for (int i = filterGuis.size() - 1; i >= 0; i--) {
        filterGuis[i]->draw();
    }

    drawFps();
}

void ofApp::setNumberSensors(int & numberSensors) {
    meatbags.setMaxCoordinateSize(numberSensors * 1440);
    
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

void ofApp::setNumberFilters(int & numberFilters) {
    if (numberFilters > filters.filters.size()) {
        while (filters.filters.size() < numberFilters) {
            addFilter(4);
        }
    }
    if (numberFilters < filters.filters.size()) {
        while (filters.filters.size() > numberFilters) {
            removeFilter();
        }
    }
}

void ofApp::addSensor() {
    int onesIndex = sensors.hokuyos.size() + 1;
    int currentIndex = sensors.hokuyos.size();
   
    float hue = fmod(currentIndex * 31.875 + 140.0, 255);
    ofColor randomColor = ofColor::fromHsb(hue, 125.0, 255.0);
    
    ofPoint center = ofPoint(0, 1.25);

    float centerRatio = float(currentIndex) / 8.0;
    float sensorX = cos(centerRatio * TWO_PI - HALF_PI) * 1.25 + center.x;
    float sensorY = sin(centerRatio * TWO_PI - HALF_PI) * 1.25 + center.y;
    
    Hokuyo* hokuyo = new Hokuyo();
    
    ofParameterGroup positionSettings;
    positionSettings.setName("position");
    positionSettings.add(hokuyo->positionX.set("position x", sensorX, -15.0, 15.0));
    positionSettings.add(hokuyo->positionY.set("position y", sensorY, 0.0, 30.0));
    positionSettings.add(hokuyo->sensorRotationDeg.set( "sensor rotation (deg)", 0, -180.0, 180.0));
    
    ofxPanel * sensorGui =  NULL;
    sensorGui = new ofxPanel();
    sensorGui->setDefaultWidth(200 - 14);
    sensorGui->setup("sensor " + to_string(onesIndex) + " settings");
    sensorGui->add(hokuyo->sensorColor.set("color", randomColor));
    sensorGui->add(hokuyo->ipAddress.set("IP address", "0.0.0.0"));
    sensorGui->add(hokuyo->autoReconnectActive.set("auto reconnect", true));
    sensorGui->add(hokuyo->mirrorAngles.set("mirror angles", false));
    sensorGui->add(hokuyo->showSensorInformation.set("show sensor info", false));
    sensorGui->add(positionSettings);
    sensorGui->getGroup("position").minimize();
    hokuyo->setInfoPosition(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
    sensors.addSensor(hokuyo);
    sensorGuis.push_back(sensorGui);
   
    float guiX = 15;
    if (currentIndex >= 4) guiX = 212;
    
    float guiY = (currentIndex % 4) * 100;
    
    sensorGuis[currentIndex]->setPosition(ofVec3f(guiX, 210 + guiY));
    
    setSpace();
}

void ofApp::addFilter(int numberPoints) {
    int onesIndex = filters.filters.size() + 1;
    int currentIndex = filters.filters.size();
    
    float centerRatio = float(currentIndex) / 15.0;
    float cx = cos(centerRatio * TWO_PI - HALF_PI) * 2.25;
    float cy = sin(centerRatio * TWO_PI - HALF_PI) * 2.25;
    
    ofPoint center = ofPoint(cx, cy + 6.0);

    if (currentIndex >= 15) center.y = cy + 12.0;
        
    Filter* filter = new Filter();
    filter->setNumberPoints(numberPoints);
    filter->index = onesIndex;
    
    ofxPanel * filterGui =  NULL;
    filterGui = new ofxPanel();
    
    ofParameterGroup coordinatesSettings;
    filterGui->setDefaultWidth(130 - 14);
    filterGui->setup("filter " + to_string(onesIndex));
    filterGui->add(filter->mask.set("mask", false));

    coordinatesSettings.setName("coordinates");
    for (int i = 0; i < numberPoints; i++) {
        float ratio = float(i) / numberPoints;
        float x = cos(ratio * TWO_PI + HALF_PI * 0.5) * 0.5 + center.x;
        float y = sin(ratio * TWO_PI + HALF_PI * 0.5) * 0.5 + center.y;
        coordinatesSettings.add(filter->points[i].set("p" + to_string(i), ofVec2f(x, y), ofVec2f(-10, 10), ofVec2f(-10, 10)));
    }
    filterGui->add(coordinatesSettings);
    filterGui->getGroup("coordinates").minimize();
    filters.addFilter(filter);
    filterGuis.push_back(filterGui);
    
    float guiX = ofGetWidth() - 126;
    if (currentIndex >= 15) guiX -= 130;
    float guiY = (currentIndex % 15) * 45;
    
    filterGuis[currentIndex]->setPosition(ofVec3f(guiX, 70 + guiY));
    
    setSpace();
}

void ofApp::removeSensor() {
    int index = sensors.hokuyos.size() - 1;
    sensorGuis.pop_back();
    sensors.removeSensor();
}

void ofApp::removeFilter() {
    int index = filters.filters.size() - 1;
    filterGuis.pop_back();
    filters.removeFilter();
}

void ofApp::drawFps() {
    std::stringstream strm;
    strm << setprecision(3) << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}

//--------------------------------------------------------------
void ofApp::exit(){
    meatbagsGui.saveToFile("generalSettings.json");
    filtersGui.saveToFile("filtersSettings.json");
    for (int i = 0; i < sensorGuis.size(); i++) {
        string filename = "sensor" + to_string(i + 1) + "Settings.json";
        sensorGuis[i]->saveToFile(filename);
    }
    
    for (int i = 0; i < filterGuis.size(); i++) {
        string filename = "filter" + to_string(i + 1) + "Settings.json";
        filterGuis[i]->saveToFile(filename);
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
    viewer.setSpace(space);
    sensors.setSpace(space);
    filters.setSpace(space);
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
