//
//  UIButton.hpp
//

#ifndef UIButton_hpp
#define UIButton_hpp

#include <stdio.h>
#include "ofMain.h"

class UIButton {
public:
    UIButton();
    
    void draw();
    void drawAdd();
    void drawRemove();
    void drawSave();
    
    void setSize(float size);
    void setPosition(float x, float y);
    void setAdd();
    void setRemove();
    void setSave();
    
    ofRectangle buttonRectangle;
    ofPolyline floppyOutline;
    ofColor buttonColor, backgroundColor;
    
    bool isMouseOver, isMouseClicked, clickLatch;
    bool isAdd, isRemove, isSave;
    float x, y, size, halfSize;
};

#endif /* UIButton_hpp */
