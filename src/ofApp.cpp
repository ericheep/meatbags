#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    headlessFont.setBold();
    headlessFont.setSize(12);
    setupGui();
    
    buttonUI.numberSensors.addListener(this, &ofApp::setNumberSensors);
    buttonUI.numberFilters.addListener(this, &ofApp::setNumberFilters);
    buttonUI.numberOscSenders.addListener(this, &ofApp::setNumberOscSenders);
    areaSize.addListener(this, &ofApp::setAreaSize);
    localIPAddress.addListener(this, &ofApp::setLocalIPAddress);
    
    setupSensorGuis();
    setupFilterGuis();
    setupOscSenderGuis();
    
    space.width = ofGetWidth();
    space.height = ofGetHeight();
    space.areaSize = areaSize;
    space.origin = ofPoint(ofGetWidth() / 2.0, 200);
    setSpace();
    
    buttonUI.setPosition(ofPoint(25, 20));
    buttonUI.onSaveCallback = std::bind(&ofApp::save, this);
    
    moveActive = false;
    setTranslation();
    
    string _localIPAddress = localIPAddress;
    setLocalIPAddress(_localIPAddress);
}

void ofApp::setupGui() {
    ofColor backgroundColor = ofColor::snow;
    backgroundColor.a = 210;
    
    ofColor barColor = ofColor::snow;
    barColor.a = 160;
    
    ofColor headerColor = ofColor::thistle;
    ofColor borderColor = ofColor::black;
    ofColor textColor = ofColor::black;
    
    ofxGuiSetDefaultHeight(12);
    ofxGuiSetDefaultWidth(230);
    ofxGuiSetBorderColor(borderColor);
    ofxGuiSetHeaderColor(headerColor);
    ofxGuiSetTextColor(textColor);
    ofxGuiSetBackgroundColor(backgroundColor);
    ofxGuiSetFillColor(barColor);
    
    hiddenGui.setup();
    hiddenGui.add(buttonUI.numberSensors.set("number sensors", 1, 1, 8));
    hiddenGui.add(buttonUI.numberFilters.set("number filters", 1, 1, 15));
    hiddenGui.add(buttonUI.numberOscSenders.set("number osc senders", 1, 1, 5));
    hiddenGui.add(areaSize.set( "area size (m)", 10.0, 0.5, 30.0));
    hiddenGui.add(translation.set("translation", ofPoint(0.0, 0.0)));
    hiddenGui.loadFromFile("hiddenSettings.json");

    meatbagsGui.setup();
    meatbagsGui.add(localIPAddress.set("local address", "0.0.0.0"));
    meatbagsGui.add(headlessMode.set("headless mode (h)", false));
    meatbagsGui.add(autoSave.set("auto save", false));
    meatbagsGui.setName("blob settings");
    meatbagsGui.add(meatbags.epsilon.set( "cluster epsilon (mm)", 100, 1, 500));
    meatbagsGui.add(meatbags.minPoints.set( "cluster min points", 10, 1, 150));
    meatbagsGui.add(meatbags.blobPersistence.set("blob persistence (s)", 0.1, 0.0, 3.0));

    meatbagsGui.setPosition(ofVec3f(15, 135, 0));
    meatbagsGui.loadFromFile("generalSettings.json");
   
    filtersGui.loadFromFile("filtersSettings.json");
}

void ofApp::setupSensorGuis() {
    int n = buttonUI.numberSensors;
    setNumberSensors(n);
    
    for (int i = 0; i < sensorGuis.size(); i++) {
        string filename = "sensor" + to_string(i + 1) + "Settings.json";
        sensorGuis[i]->loadFromFile(filename);
    }
}

void ofApp::setupFilterGuis() {
    int n = buttonUI.numberFilters;
    setNumberFilters(n);
    
    for (int i = 0; i < filterGuis.size(); i++) {
        string filename = "filter" + to_string(i + 1) + "Settings.json";
        filterGuis[i]->loadFromFile(filename);
    }
}

void ofApp::setupOscSenderGuis() {
    int n = buttonUI.numberOscSenders;
    setNumberOscSenders(n);
    
    for (int i = 0; i < oscSenderGuis.size(); i++) {
        string filename = "oscSender" + to_string(i + 1) + "Settings.json";
        oscSenderGuis[i]->loadFromFile(filename);
    }
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
    
    oscSenders.send(blobs, meatbags, sensors, filters);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    if (headlessMode) {
        headlessFont.draw("meatbags " + (string)VERSION, 15, 20);
        headlessFont.draw("headless mode", 15, 40);

        headlessFont.draw("(h) toggle headless mode / help file", 15, 80);
        headlessFont.draw("(m) hold and move mouse to translate grid", 15, 100);
        headlessFont.draw("(f) press while over the center of a filter to toggle mask", 15, 120);
       
        headlessFont.draw("blob OSC format", 15, 160);
        headlessFont.draw("/blob x y width height intensity distanceFromSensor filterIndex1 filterIndex2 ...", 15, 180);
        
        headlessFont.draw("logging OSC format", 15, 220);
        headlessFont.draw("/generalStatus sensorIndex status", 15, 240);
        headlessFont.draw("/connectionStatus sensorIndex status", 15, 260);
        headlessFont.draw("/laserStatus sensorIndex status", 15, 280);

        return;
    }
    
    viewer.draw(meatbags.oldBlobs, filters, sensors);
    buttonUI.draw();

    // draw guis
    meatbagsGui.draw();
    
    for (int i = sensorGuis.size() - 1; i >= 0; i--) {
        sensorGuis[i]->draw();
    }
    
    for (int i = filterGuis.size() - 1; i >= 0; i--) {
        filterGuis[i]->draw();
    }
    
    for (int i = oscSenderGuis.size() - 1; i >= 0; i--) {
        oscSenderGuis[i]->draw();
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
    
    setRightSideGuiPositions();
}

void ofApp::setNumberOscSenders(int & numberOscSenders) {
    if (numberOscSenders > oscSenders.oscSenders.size()) {
        while (oscSenders.oscSenders.size() < numberOscSenders) {
            addOscSender();
        }
    }
    if (numberOscSenders < oscSenders.oscSenders.size()) {
        while (oscSenders.oscSenders.size() > numberOscSenders) {
            removeOscSender();
        }
    }
    
    setRightSideGuiPositions();
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
    hokuyo->index = onesIndex;
    
    ofParameterGroup positionSettings;
    positionSettings.setName("position");
    positionSettings.add(hokuyo->positionX.set("position x", sensorX, -15.0, 15.0));
    positionSettings.add(hokuyo->positionY.set("position y", sensorY, 0.0, 30.0));
    positionSettings.add(hokuyo->sensorRotationDeg.set( "sensor rotation (deg)", 0, -180.0, 180.0));
    
    ofxPanel * sensorGui = new ofxPanel();
    sensorGui->setDefaultWidth(190);
    sensorGui->setup("sensor " + to_string(onesIndex));
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
    if (currentIndex >= 4) guiX = 210;
    
    float guiY = (currentIndex % 4) * 100;
    
    sensorGuis[currentIndex]->setPosition(ofVec3f(guiX, 233 + guiY));
    
    setSpace();
    setTranslation();
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
    
    ofxPanel * filterGui = new ofxPanel();
    
    filterGui->setDefaultWidth(130 - 14);
    filterGui->setup("filter " + to_string(onesIndex));
    filterGui->add(filter->mask.set("mask", false));

    ofParameterGroup coordinatesSettings;
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
    
    setSpace();
    setTranslation();
}

void ofApp::addOscSender() {
    int onesIndex = oscSenders.oscSenders.size() + 1;
    int currentIndex = oscSenders.oscSenders.size();
        
    OscSender* oscSender = new OscSender();
    
    ofxPanel * oscSenderGui =  NULL;
    oscSenderGui = new ofxPanel();
    
    ofParameterGroup coordinatesSettings;
    oscSenderGui->setDefaultWidth(190);
    oscSenderGui->setup("osc sender " + to_string(onesIndex));
    oscSenderGui->add(oscSender->oscSenderAddress.set("ip address", "0.0.0.0"));
    oscSenderGui->add(oscSender->oscSenderPort.set("port", 0));
    oscSenderGui->add(oscSender->sendBlobsActive.set("send blobs", false));
    oscSenderGui->add(oscSender->sendLogsActive.set("send logs", false));

    oscSenders.addOscSender(oscSender);
    oscSenderGuis.push_back(oscSenderGui);
}

void ofApp::setRightSideGuiPositions() {
    float oscSenderGuiX = ofGetWidth() - 205;
    float filterGuiX1 = ofGetWidth() - 131;
    float filterGuiX2 = ofGetWidth() - 259;

    float y = 14;
    
    for (int i = 0; i < oscSenderGuis.size(); i++) {
        oscSenderGuis[i]->setPosition(ofVec3f(oscSenderGuiX, y));
        y += 70;
    }
    
    for (int i = 0; i < filterGuis.size(); i++) {
        if (i < 8) {
            filterGuis[i]->setPosition(ofVec3f(filterGuiX1, y));
            y += 46;
        } else {
            float offset = 46 * 8;
            filterGuis[i]->setPosition(ofVec3f(filterGuiX2, y - offset));
            y += 46;
        }
    }
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

void ofApp::removeOscSender() {
    int index = oscSenders.oscSenders.size() - 1;
    oscSenderGuis.pop_back();
    oscSenders.removeOscSender();
}

void ofApp::drawFps() {
    std::stringstream strm;
    strm << setprecision(3) << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}

//--------------------------------------------------------------
void ofApp::exit() {
    if (autoSave) save();
    sensors.closeSensors();
}

void ofApp::save() {
    meatbagsGui.saveToFile("generalSettings.json");
    hiddenGui.saveToFile("hiddenSettings.json");

    for (int i = 0; i < sensorGuis.size(); i++) {
        string filename = "sensor" + to_string(i + 1) + "Settings.json";
        sensorGuis[i]->saveToFile(filename);
    }
    
    for (int i = 0; i < filterGuis.size(); i++) {
        string filename = "filter" + to_string(i + 1) + "Settings.json";
        filterGuis[i]->saveToFile(filename);
    }
    
    for (int i = 0; i < oscSenderGuis.size(); i++) {
        string filename = "oscSender" + to_string(i + 1) + "Settings.json";
        oscSenderGuis[i]->saveToFile(filename);
    }
}

void ofApp::windowResized(int width, int height) {
    for (auto& sensor : sensors.hokuyos) {
        sensor->setInfoPosition(10, height - 10);
    }
    space.width = width;
    space.height = height;
    space.origin.x = width / 2.0;
    
    setSpace();
    
    setRightSideGuiPositions();
}

void ofApp::setTranslation() {
    viewer.setTranslation(translation);
    filters.setTranslation(translation);
    sensors.setTranslation(translation);
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

void ofApp::mousePressed(int x, int y, int button) {
    if (button == 1) {
        initialTranslation = ofPoint(-x, -y) + translation;
    }
}

void ofApp::mouseMoved(int x, int y){
    if (moveActive) {
        translation = initialTranslation - ofPoint(-x, -y);
        setTranslation();
    }
}

void ofApp::mouseDragged(int x, int y, int button){
    if (button == 1) {
        translation = initialTranslation - ofPoint(-x, -y);
        setTranslation();
    }
}

void ofApp::setLocalIPAddress(string & localIPAddress) {
    sensors.setLocalIpAddress(localIPAddress);
}

void ofApp::setAreaSize(float &areaSize) {
    space.areaSize = areaSize;
    setSpace();
}

void ofApp::keyPressed(int key) {
    if (key == 104) {
        headlessMode = !headlessMode;
    }
    if (key == 109) {
        float x = ofGetMouseX();
        float y = ofGetMouseY();
        initialTranslation = ofPoint(-x, -y) + translation;

        moveActive = true;
    }
}

void ofApp::keyReleased(int key) {
    moveActive = false;
}
    
