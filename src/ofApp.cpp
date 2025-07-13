#include "ofApp.h"

string ofApp::getAppVersion() {
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>

    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFDictionaryRef infoDict = CFBundleGetInfoDictionary(mainBundle);

    CFStringRef versionStr = (CFStringRef)CFDictionaryGetValue(infoDict, CFSTR("CFBundleShortVersionString"));

    char versionBuffer[256];
    if (versionStr) {
        CFStringGetCString(versionStr, versionBuffer, sizeof(versionBuffer), kCFStringEncodingUTF8);
        return std::string(versionBuffer);
    }
    else {
        return "Unknown Version";
    }

#elif defined(_WIN32)
#include <windows.h>
#include <vector>

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    DWORD dummy;
    DWORD size = GetFileVersionInfoSizeA(exePath, &dummy);
    if (size == 0) return "Unknown Version";

    std::vector<char> buffer(size);
    if (!GetFileVersionInfoA(exePath, 0, size, buffer.data())) return "Unknown Version";

    VS_FIXEDFILEINFO* fileInfo = nullptr;
    UINT len = 0;
    if (!VerQueryValueA(buffer.data(), "\\", reinterpret_cast<LPVOID*>(&fileInfo), &len)) return "Unknown Version";

    if (fileInfo) {
        WORD major = HIWORD(fileInfo->dwFileVersionMS);
        WORD minor = LOWORD(fileInfo->dwFileVersionMS);
        WORD patch = HIWORD(fileInfo->dwFileVersionLS);
        WORD build = LOWORD(fileInfo->dwFileVersionLS);

        char versionStr[64];
        snprintf(versionStr, sizeof(versionStr), "%d.%d.%d.%d", major, minor, patch, build);
        return std::string(versionStr);
    }

    return "Unknown Version";

#else
    // For Linux or other platforms, you can hardcode or read from a config
    return "Unknown Version";
#endif
}

//--------------------------------------------------------------
void ofApp::setup(){
    version = getAppVersion();
    // ofSetFrameRate(120);
    
    titleFont.setBold();
    titleFont.setSize(14);
    helpFont.setMedium();
    helpFont.setSize(14);
    saveFont.setBold();
    saveFont.setSize(18);
    
    setupGui();
    
    buttonUI.numberSensors.addListener(this, &ofApp::setNumberSensors);
    buttonUI.numberFilters.addListener(this, &ofApp::setNumberFilters);
    buttonUI.numberOscSenders.addListener(this, &ofApp::setNumberOscSenders);
    areaSize.addListener(this, &ofApp::setAreaSize);
    interfacesDropdown.addListener(this, &ofApp::setInterface);
    
    setupMeatbagsGuis();
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
    interfacesDropdown.disableMultipleSelection();
    if (interfacesDropdown.getAllSelected().size() > 0) {
        string selection = interfacesDropdown.getAllSelected()[0];
        setInterface(selection);
    }
    
    saveNotificationTotalTime = 2.0;
    saveNotificationTimer = saveNotificationTotalTime;
    
    if (headlessMode) {
        isHelpMode = true;
        hideWindow();
    }
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
    hiddenGui.add(areaSize.set( "area size (m)", 10.0, 0.5, 50.0));
    hiddenGui.add(translation.set("translation", ofPoint(0.0, 0.0)));
    hiddenGui.loadFromFile("hiddenSettings.json");
    
    interfaceSelector.listInterfaces();
    interfacesDropdown.add(interfaceSelector.interfacesStrings);
    interfacesDropdown.setFillColor(ofColor::thistle);
    interfacesDropdown.setBackgroundColor(backgroundColor);
    interfacesDropdown.setTextColor(textColor);
    
    generalGui.setup("settings");
    generalGui.add(& interfacesDropdown);
    generalGui.add(headlessMode.set("start headless", false));
    generalGui.add(autoSave.set("auto save", false));
    generalGui.setPosition(ofVec3f(15, 130, 0));
    generalGui.loadFromFile("generalSettings.json");
}

void ofApp::setupMeatbagsGuis() {
    // int n = meatbags.numberMeatbags;
    int n = 2;
    setNumberMeatbags(n);
    
    for (int i = 0; i < meatbagsGuis.size(); i++) {
        string filename = "meatbags" + to_string(i + 1) + "Settings.json";
        meatbagsGuis[i]->loadFromFile(filename);
    }
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
    
    if (sensors.areNewCoordinatesAvailable()) {
        sensors.getCoordinates(meatbags);
        meatbags.updateBlobs();
    }
    
    meatbags.getBlobs(blobs);
    
    filters.checkBlobs(blobs);
    
    oscSenders.send(blobs, sensors, filters);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    if (!isHelpMode) drawMeatbags();
    if (isHelpMode) drawHelpText();
    
    drawSaveNotification();
    drawFps();
}

void ofApp::drawMeatbags() {
    viewer.draw(blobs, filters, sensors);
    buttonUI.draw();
    
    // draw guis
    generalGui.draw();
      
    for (int i = meatbagsGuis.size() - 1; i >= 0; i--) {
        meatbagsGuis[i]->draw();
    }
    
    for (int i = sensorGuis.size() - 1; i >= 0; i--) {
        sensorGuis[i]->draw();
    }
    
    for (int i = filterGuis.size() - 1; i >= 0; i--) {
        filterGuis[i]->draw();
    }
    
    for (int i = oscSenderGuis.size() - 1; i >= 0; i--) {
        oscSenderGuis[i]->draw();
    }
}

void ofApp::drawHelpText() {
    ofSetColor(ofColor::thistle);
    
    titleFont.draw("meatbags v" + version, 15, 20);
    helpFont.draw("headless mode", 15, 40);
    
    helpFont.draw("(h) toggle help file", 15, 80);
    helpFont.draw("(m) hold and move mouse to translate grid", 15, 100);
    helpFont.draw("(f) press while over the center of a filter to toggle mask/filter", 15, 120);
    helpFont.draw("(t) press while over the center of a filter to toggle active/inactive", 15, 140);
    helpFont.draw("(ctrl/cmd + s) press to save", 15, 160);
    
    titleFont.draw("blob OSC format", 15, 200);
    helpFont.draw("/blob index x y width length laserIntensity filterIndex1 filterIndex2 ...", 15, 220);
    helpFont.draw("/blobsActive index1 index2 ...", 15, 240);
    
    titleFont.draw("filter OSC format", 15, 280);
    helpFont.draw("/filter index isAnyBlobInside blobDistanceToCentroid", 15, 300);
    helpFont.draw("/filterBlobs filterIndex blobIndex1 x1 y1 blobIndex2 x2 y2 ...", 15, 320);
    
    titleFont.draw("logging OSC format", 15, 360);
    helpFont.draw("/generalStatus sensorIndex status", 15, 380);
    helpFont.draw("/connectionStatus sensorIndex status", 15, 400);
    helpFont.draw("/laserStatus sensorIndex status", 15, 420);
}

void ofApp::drawSaveNotification() {
    if (saveNotificationTimer < saveNotificationTotalTime) {
        string saveText = "configuration saved";
        saveNotificationTimer += ofGetLastFrameTime();
        float stringWidth = saveFont.getStringWidth(saveText);
        ofRectangle saveRectangle;
        saveRectangle.setFromCenter(ofGetWidth() * 0.5, ofGetHeight() * 0.5, stringWidth + 10, 50 - 5);
        
        ofSetColor(ofColor::black);
        ofDrawRectangle(saveRectangle);
        ofSetColor(ofColor::thistle);
        saveFont.draw(saveText, ofGetWidth() * 0.5 - stringWidth * 0.5, ofGetHeight() * 0.5);
        
    }
}

void ofApp::setInterface(string & interfaceAndIP) {
    string interface = interfaceSelector.getInterface(interfaceAndIP);
    string IP = interfaceSelector.getIP(interfaceAndIP);
    
    sensors.setInterfaceAndIP(interface, IP);
}


void ofApp::setNumberMeatbags(int& numberMeatbags) {
    int numberSensors = sensors.hokuyos.size();
    meatbags.setMaxCoordinateSize(numberSensors * 1440);
    
    if (numberMeatbags > meatbags.meatbags.size()) {
        while (meatbags.meatbags.size() < numberMeatbags) {
            addMeatbag();
        }
    }
    if (numberMeatbags < meatbags.meatbags.size()) {
        while (meatbags.meatbags.size() > numberMeatbags) {
            removeMeatbag();
        }
    }
}


void ofApp::setNumberSensors(int& numberSensors) {
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
    
    /*ofParameterGroup positionSettings;
    positionSettings.setName("position");
    positionSettings.add(hokuyo->positionX.set("position x", sensorX, -15.0, 15.0));
    positionSettings.add(hokuyo->positionY.set("position y", sensorY, 0.0, 30.0));
    positionSettings.add(hokuyo->sensorRotationDeg.set( "sensor rotation (deg)", 0, -180.0, 180.0));*/
    
    ofxPanel * sensorGui = new ofxPanel();
    sensorGui->setDefaultWidth(190);
    sensorGui->setup("sensor " + to_string(onesIndex));
    sensorGui->add(hokuyo->sensorColor.set("color", randomColor));
    sensorGui->add(hokuyo->ipAddress.set("IP address", "0.0.0.0"));
    sensorGui->add(hokuyo->autoReconnectActive.set("auto reconnect", true));
    sensorGui->add(hokuyo->mirrorAngles.set("mirror angles", false));
    sensorGui->add(hokuyo->whichMeatbag.set("which meatbag", 1, 1, 2));
    sensorGui->add(hokuyo->positionX.set("position x", sensorX, -15.0, 15.0));
    sensorGui->add(hokuyo->positionY.set("position y", sensorY, 0.0, 30.0));
    sensorGui->add(hokuyo->sensorRotationDeg.set( "sensor rotation (deg)", 0, -180.0, 180.0));
    sensorGui->add(hokuyo->showSensorInformation.set("show sensor info", false));
    
    hokuyo->setInfoPosition(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
    sensors.addSensor(hokuyo);
    sensorGuis.push_back(sensorGui);
    
    setSpace();
    setTranslation();
    setLeftSideGuiPositions();
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
    filterGui->add(filter->normalize.set("normalize", false));
    
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
    
    ofxPanel * oscSenderGui = NULL;
    oscSenderGui = new ofxPanel();
    
    // ofParameterGroup coordinatesSettings;
    oscSenderGui->setDefaultWidth(190);
    oscSenderGui->setup("osc sender " + to_string(onesIndex));
    oscSenderGui->add(oscSender->oscSenderAddress.set("ip address", "127.0.0.1"));
    oscSenderGui->add(oscSender->oscSenderPort.set("port", 5432));
    oscSenderGui->add(oscSender->sendBlobsActive.set("send blobs", false));
    oscSenderGui->add(oscSender->sendFiltersActive.set("send filters", false));
    oscSenderGui->add(oscSender->sendLogsActive.set("send logs", false));
    
    oscSenders.addOscSender(oscSender);
    oscSenderGuis.push_back(oscSenderGui);
}

void ofApp::addMeatbag() {
    int onesIndex = meatbags.meatbags.size() + 1;
    int currentIndex = meatbags.meatbags.size();
    
    Meatbags* meatbag = new Meatbags();
    meatbag->index = onesIndex;
    
    ofxPanel * meatbagGui = NULL;
    meatbagGui = new ofxPanel();
    meatbagGui = new ofxPanel();
    
    meatbagGui->setup("meatbags " + to_string(onesIndex));
    meatbagGui->add(meatbag->epsilon.set( "cluster epsilon (mm)", 100, 1, 1000));
    meatbagGui->add(meatbag->minPoints.set( "cluster min points", 10, 1, 150));
    meatbagGui->add(meatbag->blobPersistence.set("blob persistence (s)", 0.25, 0.0, 3.0));
    
    meatbags.addMeatbag(meatbag);
    meatbagsGuis.push_back(meatbagGui);
    setLeftSideGuiPositions();
}

void ofApp::setLeftSideGuiPositions() {
    float x = 15;
    float y = 210;
    
    for (int i = 0; i < meatbagsGuis.size(); i++) {
        meatbagsGuis[i]->setPosition(ofVec3f(x, y));
        y += 55;
    }
    
    for (int i = 0; i < sensorGuis.size(); i++) {
        if (i < 3) {
            sensorGuis[i]->setPosition(ofVec3f(x, y));
            y += 135;
        } else {
            float offset = 135 * 3;
            sensorGuis[i]->setPosition(ofVec3f(x + 195, y - offset));
            y += 135;
        }
    }
}

void ofApp::setRightSideGuiPositions() {
    float oscSenderGuiX = ofGetWidth() - 205;
    float filterGuiX1 = ofGetWidth() - 131;
    float filterGuiX2 = ofGetWidth() - 259;
    
    float y = 14;
    
    for (int i = 0; i < oscSenderGuis.size(); i++) {
        oscSenderGuis[i]->setPosition(ofVec3f(oscSenderGuiX, y));
        y += 86;
    }
    
    for (int i = 0; i < filterGuis.size(); i++) {
        if (i < 8) {
            filterGuis[i]->setPosition(ofVec3f(filterGuiX1, y));
            y += 56;
        } else {
            float offset = 56 * 8;
            filterGuis[i]->setPosition(ofVec3f(filterGuiX2, y - offset));
            y += 56;
        }
    }
}

void ofApp::removeMeatbag() {
    int index = meatbags.meatbags.size() - 1;
    meatbagsGuis.pop_back();
    meatbags.removeMeatbag();
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

void ofApp::hideWindow() {
#ifdef _WIN32
    // Get the native window handle (HWND)
    HWND hwnd = (HWND)ofGetWin32Window();

    // Minimize the window at startup
    ShowWindow(hwnd, SW_MINIMIZE);
#else
    std::string applescript = "osascript -e 'tell application \"System Events\" to set visible of application process \"meatbags\" to false'";
    ofSystem(applescript);
#endif
}

void ofApp::save() {
    generalGui.saveToFile("generalSettings.json");
    hiddenGui.saveToFile("hiddenSettings.json");
    
    for (int i = 0; i < meatbagsGuis.size(); i++) {
        string filename = "meatbags" + to_string(i + 1) + "Settings.json";
        meatbagsGuis[i]->saveToFile(filename);
    }
    
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
    
    saveNotificationTimer = 0;
}

void ofApp::windowResized(int width, int height) {
    for (auto& sensor : sensors.hokuyos) {
        sensor->setInfoPosition(10, height - 10);
    }
    space.width = width;
    space.height = height;
    space.origin.x = width / 2.0;
    
    setSpace();
    sensors.setInfoPositions(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
    setLeftSideGuiPositions();
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

void ofApp::setAreaSize(float &areaSize) {
    space.areaSize = areaSize;
    setSpace();
}

void ofApp::keyPressed(int key) {
    if (key == 104) {
        isHelpMode = !isHelpMode;
    }
    
    if (key == 109) {
        float x = ofGetMouseX();
        float y = ofGetMouseY();
        initialTranslation = ofPoint(-x, -y) + translation;
        
        moveActive = true;
    }
    
    if (key == 2) {
        ctrlKeyActive = true;
    }
    
    if ((ctrlKeyActive && key == 19) || key == 115) {
        save();
    }
}

void ofApp::keyReleased(int key) {
    if (key == 2) {
        ctrlKeyActive = false;
    }
    
    if (key == 109) {
        moveActive = false;
    }
}
