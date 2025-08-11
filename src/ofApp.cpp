#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    
    setupGui();
    setupListeners();
    loadConfiguration();
    
    setSpace();
    setTranslation();
    
    saveNotificationTotalTime = 2.0;
    saveNotificationTimer = saveNotificationTotalTime;
    moveActive = false;

    if (headlessMode) {
        isHelpMode = true;
        hideWindow();
    }
    
    sensorManager.start();
}

void ofApp::setupGui() {
    guiBackgroundColor = ofColor::snow;
    guiBackgroundColor.a = 210;
    guiBarColor = ofColor::snow;
    guiBarColor.a = 160;
    guiTextColor = ofColor::black;
    
    ofColor headerColor = ofColor::thistle;
    ofColor borderColor = ofColor::black;
    
    ofxGuiSetDefaultHeight(12);
    ofxGuiSetDefaultWidth(200);
    ofxGuiSetBorderColor(borderColor);
    ofxGuiSetTextColor(guiTextColor);
    ofxGuiSetBackgroundColor(guiBackgroundColor);
    ofxGuiSetFillColor(guiBarColor);
    
    hiddenGui.setup("hidden");
    hiddenGui.add(areaSize.set( "area size (m)", 10.0, 0.5, 50.0));
    hiddenGui.add(translation.set("translation", ofPoint(0.0, 0.0)));
    
    interfaceSelector.listInterfaces();
    interfacesDropdown.add(interfaceSelector.interfacesStrings);
    interfacesDropdown.setFillColor(ofColor::thistle);
    interfacesDropdown.setBackgroundColor(guiBackgroundColor);
    interfacesDropdown.setTextColor(guiTextColor);
    
    interfacesDropdown.disableMultipleSelection();
    if (interfacesDropdown.getAllSelected().size() > 0) {
        string selection = interfacesDropdown.getAllSelected()[0];
        setInterface(selection);
    }
    
    generalGui.setTextColor(guiTextColor);
    generalGui.setHeaderBackgroundColor(headerColor);
    generalGui.setup("general");
    generalGui.add(& interfacesDropdown);
    generalGui.add(headlessMode.set("start headless", false));
    generalGui.setPosition(ofVec3f(10, 135, 0));
}

void ofApp::setupListeners() {
    buttonUI.setPosition(ofPoint(25, 24));
    buttonUI.onSaveCallback = std::bind(&ofApp::save, this);
    buttonUI.onFilterAddCallback = std::bind(&ofApp::addFilter, this);
    buttonUI.onFilterRemoveCallback = std::bind(&ofApp::removeFilter, this);
    buttonUI.onSensorAddCallback = std::bind(&ofApp::addSensor, this);
    buttonUI.onSensorRemoveCallback = std::bind(&ofApp::removeSensor, this);
    buttonUI.onOscSenderAddCallback = std::bind(&ofApp::addOscSender, this);
    buttonUI.onOscSenderRemoveCallback = std::bind(&ofApp::removeOscSender, this);
    
    ofAddListener(ofEvents().mouseMoved, this, &ofApp::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &ofApp::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &ofApp::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &ofApp::onMouseReleased);
    ofAddListener(ofEvents().mouseScrolled, this, &ofApp::onMouseScrolled);
    ofAddListener(ofEvents().keyPressed, this, &ofApp::onKeyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofApp::onKeyReleased);
    
    areaSize.addListener(this, &ofApp::setAreaSize);
    interfacesDropdown.addListener(this, &ofApp::setInterface);
}

void ofApp::loadConfiguration() {
    ofJson configuration;
    ofFile file("configuration.json");
    file >> configuration;
    
    sensorManager.load(configuration);
    filterManager.load(configuration);
    oscSenderManager.load(configuration);
    meatbagsManager.load(configuration);
    
    if (configuration.contains("general")) {
        ofJson generalConfig;
        generalConfig["general"] = configuration["general"];
        generalGui.loadFrom(generalConfig);
    }
    
    if (configuration.contains("hidden")) {
        ofJson hiddenConfig;
        hiddenConfig["hidden"] = configuration["hidden"];
        hiddenGui.loadFrom(hiddenConfig);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    setTranslation();
    setSpace();
    
    filterManager.update();
    sensorManager.update();
    sensorManager.setFilters(filterManager.getFilters());
    meatbagsManager.update();
    
    //if (sensorManager.areNewCoordinatesAvailable()) {
        sensorManager.getCoordinates(meatbagsManager.getMeatbags());
        meatbagsManager.updateBlobs();
    //}
    
    meatbagsManager.getBlobs(blobs);
    filterManager.checkBlobs(blobs);
    
    oscSenderManager.send(blobs, sensorManager.getSensors(), filterManager.getFilters());
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    if (!isHelpMode) drawMeatbags();
    if (isHelpMode) viewer.drawHelpText();
    
    drawSaveNotification();
    drawFps();
}

void ofApp::drawMeatbags() {
    viewer.draw(blobs, filterManager.getFilters(), sensorManager.getSensors());
    buttonUI.draw();
    generalGui.draw();
    filterManager.draw();
    sensorManager.draw();
    oscSenderManager.draw();
    meatbagsManager.draw();
}

void ofApp::drawSaveNotification() {
    if (saveNotificationTimer < saveNotificationTotalTime) {
        viewer.drawSaveNotification();
        saveNotificationTimer += ofGetLastFrameTime();
    }
}

void ofApp::setInterface(string & interfaceAndIP) {
    string interface = interfaceSelector.getInterface(interfaceAndIP);
    string IP = interfaceSelector.getIP(interfaceAndIP);
    
    sensorManager.setInterfaceAndIP(interface, IP);
}

void ofApp::drawFps() {
    std::stringstream strm;
    strm << setprecision(3) << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}

void ofApp::exit() {
}

void ofApp::hideWindow() {
#ifdef _WIN32
    HWND hwnd = (HWND)ofGetWin32Window();
    ShowWindow(hwnd, SW_MINIMIZE);
#else
    std::string applescript = "osascript -e 'tell application \"System Events\" to set visible of application process \"meatbags\" to false'";
    ofSystem(applescript);
#endif
}

void ofApp::save() {
    ofJson configuration;
    
    generalGui.saveTo(configuration);
    hiddenGui.saveTo(configuration);
    sensorManager.saveTo(configuration);
    filterManager.saveTo(configuration);
    oscSenderManager.saveTo(configuration);
    meatbagsManager.saveTo(configuration);
    ofSavePrettyJson("configuration.json", configuration);
    
    saveNotificationTimer = 0;
}

void ofApp::addFilter() {
    filterManager.addFilter();
}

void ofApp::removeFilter() {
    filterManager.removeFilter();
}

void ofApp::addSensor() {
    sensorManager.addSensor();
}

void ofApp::removeSensor() {
    sensorManager.removeSensor();
}

void ofApp::addOscSender() {
    oscSenderManager.addOscSender();
}

void ofApp::removeOscSender() {
    oscSenderManager.removeOscSender();
}

void ofApp::windowResized(int width, int height) {
    setSpace();
    sensorManager.refreshGUIPositions();
    filterManager.refreshGUIPositions();
}

void ofApp::setTranslation() {
    viewer.setTranslation(translation);
    filterManager.setTranslation(translation);
    sensorManager.setTranslation(translation);
}

void ofApp::setSpace() {
    space.width = ofGetWidth();
    space.height = ofGetHeight();
    space.areaSize = areaSize;
    space.origin = ofPoint(ofGetWidth() / 2.0, 200);
    
    viewer.setSpace(space);
    sensorManager.setSpace(space);
    filterManager.setSpace(space);
}

void ofApp::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    viewer.onMouseMoved(mouseArgs);
    buttonUI.onMouseMoved(mouseArgs);
    sensorManager.onMouseMoved(mouseArgs);
    filterManager.onMouseMoved(mouseArgs);
    
    if (moveActive) {
        translation = initialTranslation - ofPoint(-mouseArgs.x, -mouseArgs.y);
        setTranslation();
    }
}

void ofApp::onMousePressed(ofMouseEventArgs& mouseArgs) {
    buttonUI.onMousePressed(mouseArgs);
    if (mouseArgs.button == 1) {
        initialTranslation = ofPoint(-mouseArgs.x, -mouseArgs.y) + translation;
    }
    filterManager.onMousePressed(mouseArgs);
    sensorManager.onMousePressed(mouseArgs);
}

void ofApp::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    viewer.onMouseMoved(mouseArgs);
    if (mouseArgs.button == 1) {
        translation = initialTranslation - ofPoint(-mouseArgs.x, -mouseArgs.y);
        setTranslation();
    }
    if (filterManager.onMouseDragged(mouseArgs)) return;
    sensorManager.onMouseDragged(mouseArgs);
}

void ofApp::onMouseReleased(ofMouseEventArgs&  mouseArgs) {
    buttonUI.onMouseReleased(mouseArgs);
    filterManager.onMouseReleased(mouseArgs);
    sensorManager.onMouseReleased(mouseArgs);
}

void ofApp::onMouseScrolled(ofMouseEventArgs& mouseArgs) {
    areaSize -= mouseArgs.scrollY * 0.15;
    areaSize = ofClamp(areaSize, areaSize.getMin(), areaSize.getMax());
}

void ofApp::onKeyPressed(ofKeyEventArgs& keyArgs) {
    filterManager.onKeyPressed(keyArgs);
    
    if (keyArgs.key == 2) ctrlKeyActive = true;
    if (keyArgs.key == 104) isHelpMode = !isHelpMode;
    if ((ctrlKeyActive && keyArgs.key == 19) || keyArgs.key == 115) save();
    
    if (keyArgs.key == 109) {
        float x = ofGetMouseX();
        float y = ofGetMouseY();
        initialTranslation = ofPoint(-x, -y) + translation;
        
        moveActive = true;
    }
}

void ofApp::onKeyReleased(ofKeyEventArgs& keyArgs) {
    if (keyArgs.key == 2) ctrlKeyActive = false;
    if (keyArgs.key == 109) moveActive = false;
}

void ofApp::setAreaSize(float &areaSize) {
    space.areaSize = areaSize;
    setSpace();
}
