//
//  UI.cpp
//

#include "UI.hpp"
#include <functional>

UI::UI() {
	size   = 16;
	offset = 5;

	float buttonSpacing = size + offset;

	// sizes
	saveButton.setSize(size);
	addSensorButton.setSize(size);
	removeSensorButton.setSize(size);
	addFilterButton.setSize(size);
	removeFilterButton.setSize(size);
	addOscSenderButton.setSize(size);
	removeOscSenderButton.setSize(size);
	addMeatbagButton.setSize(size);
	removeMeatbagButton.setSize(size);

	// types
	saveButton.setSave();
	addSensorButton.setAdd();
	removeSensorButton.setRemove();
	addFilterButton.setAdd();
	removeFilterButton.setRemove();
	addOscSenderButton.setAdd();
	removeOscSenderButton.setRemove();
	addMeatbagButton.setAdd();
	removeMeatbagButton.setRemove();

	// positions — meatbags row 1, sensors row 2, filters row 3, osc senders row 4, save row 5
	addMeatbagButton.setPosition(0,             buttonSpacing * 1);
	removeMeatbagButton.setPosition(buttonSpacing, buttonSpacing * 1);

	addSensorButton.setPosition(0,             buttonSpacing * 2);
	removeSensorButton.setPosition(buttonSpacing, buttonSpacing * 2);

	addFilterButton.setPosition(0,             buttonSpacing * 3);
	removeFilterButton.setPosition(buttonSpacing, buttonSpacing * 3);

	addOscSenderButton.setPosition(0,             buttonSpacing * 4);
	removeOscSenderButton.setPosition(buttonSpacing, buttonSpacing * 4);

	saveButton.setPosition(0, buttonSpacing * 5.25);

	uiFont.setBold();
	uiFont.setSize(13);
	titleFont.setBold();
	titleFont.setSize(15);
}

UI::~UI() {}

void UI::draw() {
	ofPushMatrix();

	ofRectangle backgroundRectangle;
	float buttonSpacing = size + offset;
	float saveY         = saveButton.y + size * 0.5f + 8.0f;  // bottom of save button + padding
	float bgHeight      = saveY + size + 8.0f;
	backgroundRectangle.set(position.x - 15, position.y - 18, 220, bgHeight);

	ofColor transparentBlack = ofColor::black;
	transparentBlack.a = 150;
	ofSetColor(transparentBlack);
	ofFill();
	ofDrawRectangle(backgroundRectangle);

	ofSetColor(ofColor::thistle);
	ofNoFill();
	ofDrawRectangle(backgroundRectangle);

	ofTranslate(position.x, position.y);

	saveButton.draw();
	addMeatbagButton.draw();
	removeMeatbagButton.draw();
	addSensorButton.draw();
	removeSensorButton.draw();
	addFilterButton.draw();
	removeFilterButton.draw();
	addOscSenderButton.draw();
	removeOscSenderButton.draw();

	ofSetColor(ofColor::thistle);

	titleFont.draw("meatbags",   -8,  2);
	uiFont.draw("meatbags",      35,  addMeatbagButton.y   + 5);
	uiFont.draw("sensors",       35,  addSensorButton.y    + 5);
	uiFont.draw("filters",       35,  addFilterButton.y    + 5);
	uiFont.draw("osc senders",   35,  addOscSenderButton.y + 5);
	uiFont.draw("save",          14,  saveButton.y         + 5);

	ofPopMatrix();
}

void UI::setPosition(ofPoint _position) {
	position = _position;
}

float UI::getBottom() {
	// matches the backgroundRectangle in draw():
	// set(position.x - 15, position.y - 18, 220, h)
	float buttonSpacing = size + offset;
	float saveY         = position.y + buttonSpacing * 5.25f;
	float bgTop         = position.y - 18.0f;
	float bgHeight      = (saveY + size) - bgTop + 8.0f;  // 8px bottom padding
	return bgTop + bgHeight;
}

void UI::addSensor()      { if (onSensorAddCallback)      onSensorAddCallback(); }
void UI::removeSensor()   { if (onSensorRemoveCallback)   onSensorRemoveCallback(); }
void UI::addFilter()      { if (onFilterAddCallback)      onFilterAddCallback(); }
void UI::removeFilter()   { if (onFilterRemoveCallback)   onFilterRemoveCallback(); }
void UI::addOscSender()   { if (onOscSenderAddCallback)   onOscSenderAddCallback(); }
void UI::removeOscSender(){ if (onOscSenderRemoveCallback) onOscSenderRemoveCallback(); }
void UI::addMeatbag()     { if (onMeatbagAddCallback)     onMeatbagAddCallback(); }
void UI::removeMeatbag()  { if (onMeatbagRemoveCallback)  onMeatbagRemoveCallback(); }

void UI::checkIfMouseOver(UIButton& button, ofPoint mousePoint) {
	button.isMouseOver = mousePoint.distance(ofPoint(button.x, button.y) + position) <= button.halfSize;
}

void UI::checkIfMouseClicked(UIButton& button, ofPoint mousePoint) {
	button.isMouseClicked = mousePoint.distance(ofPoint(button.x, button.y) + position) <= button.halfSize;
}

void UI::checkIfMouseReleased(UIButton& button) {
	if (button.isMouseClicked) {
		button.isMouseClicked = false;
		button.clickLatch     = true;
	}
}

void UI::onMouseMoved(ofMouseEventArgs& mouseArgs) {
	ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
	checkIfMouseOver(saveButton,            mousePoint);
	checkIfMouseOver(addMeatbagButton,      mousePoint);
	checkIfMouseOver(removeMeatbagButton,   mousePoint);
	checkIfMouseOver(addSensorButton,       mousePoint);
	checkIfMouseOver(removeSensorButton,    mousePoint);
	checkIfMouseOver(addFilterButton,       mousePoint);
	checkIfMouseOver(removeFilterButton,    mousePoint);
	checkIfMouseOver(addOscSenderButton,    mousePoint);
	checkIfMouseOver(removeOscSenderButton, mousePoint);
}

void UI::onMousePressed(ofMouseEventArgs& mouseArgs) {
	ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

	checkIfMouseClicked(saveButton,            mousePoint);
	checkIfMouseClicked(addMeatbagButton,      mousePoint);
	checkIfMouseClicked(removeMeatbagButton,   mousePoint);
	checkIfMouseClicked(addSensorButton,       mousePoint);
	checkIfMouseClicked(removeSensorButton,    mousePoint);
	checkIfMouseClicked(addFilterButton,       mousePoint);
	checkIfMouseClicked(removeFilterButton,    mousePoint);
	checkIfMouseClicked(addOscSenderButton,    mousePoint);
	checkIfMouseClicked(removeOscSenderButton, mousePoint);

	if (saveButton.isMouseClicked && saveButton.clickLatch) {
		if (onSaveCallback) onSaveCallback();
		saveButton.clickLatch = false;
	}
	if (addMeatbagButton.isMouseClicked && addMeatbagButton.clickLatch) {
		addMeatbag();
		addMeatbagButton.clickLatch = false;
	}
	if (removeMeatbagButton.isMouseClicked && removeMeatbagButton.clickLatch) {
		removeMeatbag();
		removeMeatbagButton.clickLatch = false;
	}
	if (addSensorButton.isMouseClicked && addSensorButton.clickLatch) {
		addSensor();
		addSensorButton.clickLatch = false;
	}
	if (removeSensorButton.isMouseClicked && removeSensorButton.clickLatch) {
		removeSensor();
		removeSensorButton.clickLatch = false;
	}
	if (addFilterButton.isMouseClicked && addFilterButton.clickLatch) {
		addFilter();
		addFilterButton.clickLatch = false;
	}
	if (removeFilterButton.isMouseClicked && removeFilterButton.clickLatch) {
		removeFilter();
		removeFilterButton.clickLatch = false;
	}
	if (addOscSenderButton.isMouseClicked && addOscSenderButton.clickLatch) {
		addOscSender();
		addOscSenderButton.clickLatch = false;
	}
	if (removeOscSenderButton.isMouseClicked && removeOscSenderButton.clickLatch) {
		removeOscSender();
		removeOscSenderButton.clickLatch = false;
	}
}

void UI::onMouseReleased(ofMouseEventArgs& mouseArgs) {
	checkIfMouseReleased(saveButton);
	checkIfMouseReleased(addMeatbagButton);
	checkIfMouseReleased(removeMeatbagButton);
	checkIfMouseReleased(addSensorButton);
	checkIfMouseReleased(removeSensorButton);
	checkIfMouseReleased(addFilterButton);
	checkIfMouseReleased(removeFilterButton);
	checkIfMouseReleased(addOscSenderButton);
	checkIfMouseReleased(removeOscSenderButton);
}
