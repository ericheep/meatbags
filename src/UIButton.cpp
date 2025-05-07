//
//  UIButton.cpp
//

#include "UIButton.hpp"

UIButton::UIButton() {
    isMouseOver = false;
    isMouseClicked = false;
    clickLatch = true;
    
    buttonColor = ofColor::thistle;
    backgroundColor = ofColor::black;    
}

void UIButton::setSize(float _size) {
    size = _size;
    halfSize = _size * 0.5;
    
    buttonRectangle.setFromCenter(x, y, size, size);
   
}

void UIButton::setPosition(float _x, float _y) {
    x = _x;
    y = _y;
    
    buttonRectangle.setFromCenter(x, y, size, size);
    
    floppyOutline.addVertex(ofPoint(x + size * 0.5, y - size * 0.4));
    floppyOutline.addVertex(ofPoint(x + size * 0.5, y + size * 0.5));
    floppyOutline.addVertex(ofPoint(x - size * 0.5, y + size * 0.5));
    floppyOutline.addVertex(ofPoint(x - size * 0.5, y - size * 0.5));
    floppyOutline.addVertex(ofPoint(x + size * 0.4, y - size * 0.5));
    floppyOutline.close();
}

void UIButton::setAdd() {
    isAdd = true;
    isRemove = false;
    isSave = false;
}

void UIButton::setRemove() {
    isAdd = false;
    isRemove = true;
    isSave = false;
}

void UIButton::setSave() {
    isAdd = false;
    isRemove = false;
    isSave = true;
}

void UIButton::draw() {
    if (isMouseOver) {
        buttonColor = ofColor::black;
        backgroundColor = ofColor::thistle;
    } else {
        buttonColor = ofColor::thistle;
        backgroundColor = ofColor::black;
    }
    
   
    if (isAdd) {
        drawAdd();
    } else if (isRemove) {
        drawRemove();
    } else if (isSave) {
        drawSave();
    }
}

void UIButton::drawAdd() {
    float iconSize = size;
    if (isMouseClicked) {
        iconSize = iconSize * 0.7;
    }
    
    ofFill();
    ofSetColor(backgroundColor);
    ofDrawRectangle(buttonRectangle);
    
    ofNoFill();
    ofSetColor(buttonColor);
    ofDrawRectangle(buttonRectangle);
    
    ofRectangle verticalBar;
    verticalBar.setFromCenter(x, y, iconSize * .13, iconSize  * .62);
    
    ofRectangle horizontalBar;
    horizontalBar.setFromCenter(x, y, iconSize * .62, iconSize  * .13);
    
    ofFill();
    ofDrawRectangle(verticalBar);
    ofDrawRectangle(horizontalBar);
}

void UIButton::drawRemove() {
    float iconSize = size;
    if (isMouseClicked) {
        iconSize = iconSize * 0.7;
    }
    
    ofFill();
    ofSetColor(backgroundColor);
    ofDrawRectangle(buttonRectangle);
    
    ofNoFill();
    ofSetColor(buttonColor);
    ofDrawRectangle(buttonRectangle);
    
    ofRectangle horizontalBar;
    horizontalBar.setFromCenter(x, y, iconSize * .62, iconSize  * .13);
    
    ofFill();
    ofDrawRectangle(horizontalBar);
}

void UIButton::drawSave() {
    float iconSize = size;
    if (isMouseClicked) {
        iconSize = iconSize * 0.9;
    }
    
    ofFill();
    ofSetColor(backgroundColor);
    ofDrawRectangle(buttonRectangle);
    
    ofNoFill();
    ofSetColor(buttonColor);
    floppyOutline.draw();
    
    ofRectangle verticalBar;
    verticalBar.setFromCenter(x, y, iconSize * .13, iconSize  * .62);
    
    ofRectangle horizontalBar;
    horizontalBar.setFromCenter(x, y, iconSize * .62, iconSize  * .13);
    
    ofFill();
    ofSetColor(buttonColor);
    ofRectangle top;
    top.setFromCenter(x - iconSize * .0, y - iconSize * 0.3, iconSize * 0.6, iconSize * 0.4);
    ofDrawRectangle(top);
    
    ofRectangle metal;
    metal.setFromCenter(x, y + 0.25 * iconSize, iconSize * .8, iconSize * 0.4);
    ofDrawRectangle(metal);
    
    ofSetColor(backgroundColor);
    ofRectangle topCutout;
    topCutout.setFromCenter(x + iconSize * 0.1, y - iconSize * 0.3, iconSize * 0.2, iconSize * 0.3);
    ofDrawRectangle(topCutout);
    
    for (int i = 0; i < 3; i++) {
        ofRectangle stripe;
        float yOffset = ofMap(i, 0, 2, 0.15, 0.35) * iconSize;
        stripe.setFromCenter(x, y + yOffset, iconSize * .62, iconSize  * .05);
        ofDrawRectangle(stripe);
    }
}

