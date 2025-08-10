//
//  UI.cpp
//

#include "UI.hpp"
#include <functional>

UI::UI() {
    size = 16;
    offset = 5;

    // set sizes
    saveButton.setSize(size);
    
    addSensorButton.setSize(size);
    removeSensorButton.setSize(size);
    
    addFilterButton.setSize(size);
    removeFilterButton.setSize(size);
    
    addOscSenderButton.setSize(size);
    removeOscSenderButton.setSize(size);

    // set + or - or save
    saveButton.setSave();
    
    addSensorButton.setAdd();
    removeSensorButton.setRemove();
    
    addFilterButton.setAdd();
    removeFilterButton.setRemove();
    
    addOscSenderButton.setAdd();
    removeOscSenderButton.setRemove();

    // set positions
    float buttonSpacing = size + offset;
    
    addSensorButton.setPosition(0, buttonSpacing * 1);
    removeSensorButton.setPosition(buttonSpacing, buttonSpacing * 1);
    
    addFilterButton.setPosition(0, buttonSpacing * 2);
    removeFilterButton.setPosition(buttonSpacing, buttonSpacing * 2);
    
    addOscSenderButton.setPosition(0, buttonSpacing * 3);
    removeOscSenderButton.setPosition(buttonSpacing, buttonSpacing * 3);
    
    saveButton.setPosition(0, buttonSpacing * 4.25);
    
    uiFont.setBold();
    uiFont.setSize(13);
    titleFont.setBold();
    titleFont.setSize(15);
}

UI::~UI() {}

void UI::draw() {
    ofPushMatrix();
    
    ofTranslate(position.x, position.y);
    saveButton.draw();
    addSensorButton.draw();
    removeSensorButton.draw();
    addFilterButton.draw();
    removeFilterButton.draw();
    addOscSenderButton.draw();
    removeOscSenderButton.draw();
    
    ofSetColor(ofColor::thistle);
        
    titleFont.draw("meatbags", -8, -1);
    uiFont.draw("sensors", 35, addSensorButton.y + 5);
    uiFont.draw("filters ", 35, addFilterButton.y + 5);
    uiFont.draw("osc senders", 35, addOscSenderButton.y + 5);
    uiFont.draw("save", 14, saveButton.y + 5);
    ofPopMatrix();
}

void UI::setPosition(ofPoint _position) {
    position = _position;
}

void UI::addSensor() {
    onSensorAddCallback();
}

void UI::removeSensor() {
    onSensorRemoveCallback();
}

void UI::addFilter() {
    onFilterAddCallback();
}

void UI::removeFilter() {
    onFilterRemoveCallback();
}

void UI::addOscSender() {
    onOscSenderAddCallback();
}

void UI::removeOscSender() {
    onOscSenderRemoveCallback();
}

void UI::checkIfMouseOver(UIButton & button, ofPoint mousePoint) {
    if(mousePoint.distance(ofPoint(button.x, button.y) + position) <= button.halfSize) {
        button.isMouseOver = true;
    } else {
        button.isMouseOver = false;
    }
}

void UI::checkIfMouseClicked(UIButton & button, ofPoint mousePoint) {
    if(mousePoint.distance(ofPoint(button.x, button.y) + position) <= button.halfSize) {
        button.isMouseClicked = true;
    } else {
        button.isMouseClicked = false;
    }
}

void UI::checkIfMouseReleased(UIButton & button) {
    if (button.isMouseClicked) {
        button.isMouseClicked = false;
        button.clickLatch = true;
    }
}

void UI::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    checkIfMouseOver(saveButton, mousePoint);
    checkIfMouseOver(addSensorButton, mousePoint);
    checkIfMouseOver(removeSensorButton, mousePoint);
    checkIfMouseOver(addFilterButton, mousePoint);
    checkIfMouseOver(removeFilterButton, mousePoint);
    checkIfMouseOver(addOscSenderButton, mousePoint);
    checkIfMouseOver(removeOscSenderButton, mousePoint);
}

void UI::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    checkIfMouseClicked(saveButton, mousePoint);
    checkIfMouseClicked(addSensorButton, mousePoint);
    checkIfMouseClicked(removeSensorButton, mousePoint);
    checkIfMouseClicked(addFilterButton, mousePoint);
    checkIfMouseClicked(removeFilterButton, mousePoint);
    checkIfMouseClicked(addOscSenderButton, mousePoint);
    checkIfMouseClicked(removeOscSenderButton, mousePoint);
    
    if (saveButton.isMouseClicked && saveButton.clickLatch) {
        if (onSaveCallback) {
            onSaveCallback();
        }
        saveButton.clickLatch = false;
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
    checkIfMouseReleased(addSensorButton);
    checkIfMouseReleased(removeSensorButton);
    checkIfMouseReleased(addFilterButton);
    checkIfMouseReleased(removeFilterButton);
    checkIfMouseReleased(addOscSenderButton);
    checkIfMouseReleased(removeOscSenderButton);
}
