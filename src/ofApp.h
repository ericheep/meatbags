#pragma once

#include "ofMain.h"
#include "ofJson.h"

#include "SensorManager.hpp"
#include "FilterManager.hpp"
#include "MeatbagsManager.hpp"
#include "OscSenderManager.hpp"
#include "Viewer.hpp"
#include "GeneralPanel.hpp"
#include "MeatbagsPanel.hpp"
#include "SensorPanel.hpp"
#include "OscSenderPanel.hpp"
#include "FilterPanel.hpp"

#include "UI.hpp"
#include "Space.h"

class ofApp : public ofBaseApp{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void exit() override;
	void windowResized(int width, int height) override;
	
	void onMouseMoved(ofMouseEventArgs & mouseArgs);
	void onMousePressed(ofMouseEventArgs & mouseArgs);
	void onMouseDragged(ofMouseEventArgs & mouseArgs);
	void onMouseReleased(ofMouseEventArgs & mouseArgs);
	void onMouseScrolled(ofMouseEventArgs & mouseArgs);
	void onKeyPressed(ofKeyEventArgs & keyArgs);
	void onKeyReleased(ofKeyEventArgs & keyArgs);
	
	void drawMeatbags();
	void drawHelpText();
	void drawSaveNotification();
	
	void drawFps();
	void save();
	void hideWindow();
	
	void setupGui();
	void setupListeners();
	void loadConfiguration();
	
	void setSpace();
	void setTranslation();
	
	void addMeatbag();
	void removeMeatbag();
	
	void addSensor();
	void removeSensor();
	
	void addFilter();
	void removeFilter();
	
	void addOscSender();
	void removeOscSender();
	
	void setAreaSize(float &areaSize);

	SensorManager    sensorManager;
	FilterManager    filterManager;
	OscSenderManager oscSenderManager;
	MeatbagsManager  meatbagsManager;

	GeneralPanel     generalPanel;
	MeatbagsPanel    meatbagsPanel;
	SensorPanel      sensorPanel;
	OscSenderPanel   oscSenderPanel;
	FilterPanel      filterPanel;

	vector<Blob> blobs;
	Viewer       viewer;
	UI           buttonUI;
	Space        space;

	ofPoint initialTranslation;

	ofParameter<bool>    headlessMode;
	ofParameter<ofPoint> translation;
	ofParameter<string>  localIPAddress;
	ofParameter<float>   areaSize;

	float saveNotificationTimer, saveNotificationTotalTime;
	bool  moveActive, ctrlKeyActive, isHelpMode;
};
