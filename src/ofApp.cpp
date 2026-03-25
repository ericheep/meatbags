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
	// initialize camera state parameters directly
	areaSize.set("area size (m)", 10.0, 0.5, 50.0);
	translation.set("translation", ofPoint(0.0, 0.0));

	// generalPanel
	generalPanel.x = 10;
	generalPanel.y = 135;
	generalPanel.setup();
	generalPanel.onInterfaceChanged = [this](string iface, string localIP) {
		sensorManager.setInterfaceAndIP(iface, localIP);
	};
	generalPanel.onHeadlessModeChanged = [this](bool h) {
		headlessMode = h;
	};

	// meatbagsPanel and sensorPanel positions set dynamically in drawMeatbags
	meatbagsPanel.x = 10;
	sensorPanel.x   = 10;

	// oscSenderPanel and filterPanel on right side
	oscSenderPanel.y = 10;
	// filterPanel.y set dynamically in drawMeatbags
	filterPanel.onFilterTypeChanged = [this](int index, string type) {
		FilterType ft = (type == "Quad") ? FilterType::Quad : FilterType::Ellipse;
		filterManager.changeFilterType(index, ft);
	};
	sensorPanel.onSensorTypeChanged = [this](int index, string type) {
		sensorManager.changeSensorType(index, sensorManager.stringToSensorType(type));
	};
}

void ofApp::setupListeners() {
	buttonUI.setPosition(ofPoint(25, 24));
	buttonUI.onSaveCallback             = std::bind(&ofApp::save, this);
	buttonUI.onFilterAddCallback        = std::bind(&ofApp::addFilter, this);
	buttonUI.onFilterRemoveCallback     = std::bind(&ofApp::removeFilter, this);
	buttonUI.onSensorAddCallback        = std::bind(&ofApp::addSensor, this);
	buttonUI.onSensorRemoveCallback     = std::bind(&ofApp::removeSensor, this);
	buttonUI.onOscSenderAddCallback     = std::bind(&ofApp::addOscSender, this);
	buttonUI.onOscSenderRemoveCallback  = std::bind(&ofApp::removeOscSender, this);

	ofAddListener(ofEvents().mouseMoved,    this, &ofApp::onMouseMoved);
	ofAddListener(ofEvents().mousePressed,  this, &ofApp::onMousePressed);
	ofAddListener(ofEvents().mouseDragged,  this, &ofApp::onMouseDragged);
	ofAddListener(ofEvents().mouseReleased, this, &ofApp::onMouseReleased);
	ofAddListener(ofEvents().mouseScrolled, this, &ofApp::onMouseScrolled);
	ofAddListener(ofEvents().keyPressed,    this, &ofApp::onKeyPressed);
	ofAddListener(ofEvents().keyReleased,   this, &ofApp::onKeyReleased);

	areaSize.addListener(this, &ofApp::setAreaSize);
}

void ofApp::loadConfiguration() {
	ofJson configuration;

	try {
		ofFile file("configuration.json");
		if (file.exists()) {
			file >> configuration;

			sensorManager.load(configuration);
			filterManager.load(configuration);
			oscSenderManager.load(configuration);
			meatbagsManager.load(configuration);

			generalPanel.loadFrom(configuration);
			meatbagsPanel.loadFrom(configuration);
			sensorPanel.loadFrom(configuration);
			oscSenderPanel.loadFrom(configuration);
			filterPanel.loadFrom(configuration);

			if (configuration.contains("hidden")) {
				ofJson h = configuration["hidden"];
				if (h.contains("area size (m)"))  areaSize  = h["area size (m)"].get<float>();
				if (h.contains("translation")) {
					ofJson t = h["translation"];
					translation = ofPoint(t.value("x", 0.0f), t.value("y", 0.0f));
				}
			}

			return;
		}
	}
	catch (int errorCode) {
	}

	sensorManager.initialize();
	filterManager.initialize();
	oscSenderManager.initialize();
	meatbagsManager.initialize();
}

//--------------------------------------------------------------
void ofApp::update(){
	setTranslation();
	setSpace();

	filterManager.update();
	sensorManager.update();
	sensorManager.setFilters(filterManager.getFilters());
	meatbagsManager.update();

	if (sensorManager.areNewCoordinatesAvailable()) {
		sensorManager.getCoordinates(meatbagsManager.getMeatbags());
		meatbagsManager.updateBlobs();
	}

	meatbagsManager.getBlobs(blobs);
	filterManager.checkBlobs(blobs);

	oscSenderManager.send(blobs, sensorManager.getSensors(), filterManager.getFilters());
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);

	if (!isHelpMode) drawMeatbags();
	if (isHelpMode)  viewer.drawHelpText();

	drawSaveNotification();
	drawFps();
}

void ofApp::drawMeatbags() {
	viewer.drawCoordinates(sensorManager.lidarPoints, sensorManager.numberLidarPoints);
	viewer.draw(blobs, filterManager.getFilters(), sensorManager.getSensors());
	buttonUI.draw();
	// position panels dynamically with consistent margin
	int margin = 4;
	meatbagsPanel.y = generalPanel.y + generalPanel.totalHeight() + margin;
	sensorPanel.y   = meatbagsPanel.y + meatbagsPanel.totalHeight(meatbagsManager.getMeatbags().size()) + margin;
	meatbagsPanel.draw(meatbagsManager.getMeatbags());
	sensorPanel.draw(sensorManager.getSensors());
	// oscSenderPanel right side, dynamic x
	int rightMargin = 10;
	oscSenderPanel.x = ofGetWidth() - oscSenderPanel.width - rightMargin;
	oscSenderPanel.draw(oscSenderManager.getOscSenders());
	filterPanel.x = ofGetWidth() - filterPanel.width - rightMargin;
	filterPanel.y = oscSenderPanel.y + oscSenderPanel.totalHeight(oscSenderManager.getOscSenders().size()) + 4;
	filterPanel.draw(filterManager.getFilters());
	generalPanel.draw();
}

void ofApp::drawSaveNotification() {
	if (saveNotificationTimer < saveNotificationTotalTime) {
		viewer.drawSaveNotification();
		saveNotificationTimer += ofGetLastFrameTime();
	}
}

void ofApp::drawFps() {
	std::stringstream strm;
	strm << std::setprecision(3) << "fps: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());
}

void ofApp::exit() {
	ofRemoveListener(ofEvents().mouseMoved,    this, &ofApp::onMouseMoved);
	ofRemoveListener(ofEvents().mousePressed,  this, &ofApp::onMousePressed);
	ofRemoveListener(ofEvents().mouseDragged,  this, &ofApp::onMouseDragged);
	ofRemoveListener(ofEvents().mouseReleased, this, &ofApp::onMouseReleased);
	ofRemoveListener(ofEvents().mouseScrolled, this, &ofApp::onMouseScrolled);
	ofRemoveListener(ofEvents().keyPressed,    this, &ofApp::onKeyPressed);
	ofRemoveListener(ofEvents().keyReleased,   this, &ofApp::onKeyReleased);

	areaSize.removeListener(this, &ofApp::setAreaSize);
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

	generalPanel.saveTo(configuration);
	meatbagsPanel.saveTo(configuration);
	sensorPanel.saveTo(configuration);
	oscSenderPanel.saveTo(configuration);
	filterPanel.saveTo(configuration);
	// save camera state manually
	configuration["hidden"]["area size (m)"] = areaSize.get();
	configuration["hidden"]["translation"]["x"] = translation.get().x;
	configuration["hidden"]["translation"]["y"] = translation.get().y;
	sensorManager.saveTo(configuration);
	filterManager.saveTo(configuration);
	oscSenderManager.saveTo(configuration);
	meatbagsManager.saveTo(configuration);
	ofSavePrettyJson("configuration.json", configuration);

	saveNotificationTimer = 0;
}

void ofApp::addFilter()          { filterManager.addFilter();    filterPanel.scrollY = 0.0f; }
void ofApp::removeFilter()       { filterManager.removeFilter(); filterPanel.scrollY = 0.0f; }
void ofApp::addSensor()       { sensorManager.addSensor();    sensorPanel.scrollY = 0.0f; }
void ofApp::removeSensor()    { sensorManager.removeSensor(); sensorPanel.scrollY = 0.0f; }
void ofApp::addOscSender()       { oscSenderManager.addOscSender(); }
void ofApp::removeOscSender()    { oscSenderManager.removeOscSender(); }

void ofApp::windowResized(int width, int height) {
	setSpace();
}

void ofApp::setTranslation() {
	viewer.setTranslation(translation);
	filterManager.setTranslation(translation);
	sensorManager.setTranslation(translation);
}

void ofApp::setSpace() {
	space.width    = ofGetWidth();
	space.height   = ofGetHeight();
	space.areaSize = areaSize;
	space.origin   = ofPoint(ofGetWidth() / 2.0, 200);

	viewer.setSpace(space);
	sensorManager.setSpace(space);
	filterManager.setSpace(space);
}

void ofApp::onMouseMoved(ofMouseEventArgs& mouseArgs) {
	viewer.onMouseMoved(mouseArgs);
	buttonUI.onMouseMoved(mouseArgs);
	generalPanel.onMouseMoved(mouseArgs);
	meatbagsPanel.onMouseMoved(mouseArgs, meatbagsManager.getMeatbags());
	sensorPanel.onMouseMoved(mouseArgs, sensorManager.getSensors());
	oscSenderPanel.onMouseMoved(mouseArgs, oscSenderManager.getOscSenders());
	filterPanel.onMouseMoved(mouseArgs, filterManager.getFilters());
	sensorManager.onMouseMoved(mouseArgs);
	filterManager.onMouseMoved(mouseArgs);

	if (moveActive) {
		translation = initialTranslation - ofPoint(-mouseArgs.x, -mouseArgs.y);
		setTranslation();
	}
}

void ofApp::onMousePressed(ofMouseEventArgs& mouseArgs) {
	buttonUI.onMousePressed(mouseArgs);
	if (generalPanel.onMousePressed(mouseArgs)) return;
	if (meatbagsPanel.onMousePressed(mouseArgs, meatbagsManager.getMeatbags())) return;
	if (sensorPanel.onMousePressed(mouseArgs, sensorManager.getSensors())) return;
	if (oscSenderPanel.onMousePressed(mouseArgs, oscSenderManager.getOscSenders())) return;
	if (filterPanel.onMousePressed(mouseArgs, filterManager.getFilters())) return;
	if (mouseArgs.button == 1) {
		initialTranslation = ofPoint(-mouseArgs.x, -mouseArgs.y) + translation;
	}
	filterManager.onMousePressed(mouseArgs);
	sensorManager.onMousePressed(mouseArgs);
}

void ofApp::onMouseDragged(ofMouseEventArgs& mouseArgs) {
	viewer.onMouseMoved(mouseArgs);
	meatbagsPanel.onMouseDragged(mouseArgs, meatbagsManager.getMeatbags());
	sensorPanel.onMouseDragged(mouseArgs, sensorManager.getSensors());
	if (mouseArgs.button == 1) {
		translation = initialTranslation - ofPoint(-mouseArgs.x, -mouseArgs.y);
		setTranslation();
	}
	if (filterManager.onMouseDragged(mouseArgs)) return;
	sensorManager.onMouseDragged(mouseArgs);
}

void ofApp::onMouseReleased(ofMouseEventArgs& mouseArgs) {
	buttonUI.onMouseReleased(mouseArgs);
	meatbagsPanel.onMouseReleased(mouseArgs);
	sensorPanel.onMouseReleased(mouseArgs);
	filterPanel.onMouseReleased(mouseArgs);
	filterManager.onMouseReleased(mouseArgs);
	sensorManager.onMouseReleased(mouseArgs);
}

void ofApp::onMouseScrolled(ofMouseEventArgs& mouseArgs) {
	if (sensorPanel.onMouseScrolled(mouseArgs, sensorManager.getSensors())) return;
	if (filterPanel.onMouseScrolled(mouseArgs, filterManager.getFilters())) return;
	areaSize -= mouseArgs.scrollY * 0.15;
	areaSize = ofClamp(areaSize, areaSize.getMin(), areaSize.getMax());
}

void ofApp::onKeyPressed(ofKeyEventArgs& keyArgs) {
	if (sensorPanel.onKeyPressed(keyArgs, sensorManager.getSensors())) return;
	if (oscSenderPanel.onKeyPressed(keyArgs, oscSenderManager.getOscSenders())) return;
	if (meatbagsPanel.onKeyPressed(keyArgs, meatbagsManager.getMeatbags())) return;
	filterManager.onKeyPressed(keyArgs);

	if (keyArgs.key == 2)   ctrlKeyActive = true;
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
	if (keyArgs.key == 2)   ctrlKeyActive = false;
	if (keyArgs.key == 109) moveActive = false;
}

void ofApp::setAreaSize(float& areaSize) {
	space.areaSize = areaSize;
	setSpace();
}
