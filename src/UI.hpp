//
//  UI.hpp
//

#ifndef UI_hpp
#define UI_hpp

#include <stdio.h>
#include "ofMain.h"
#include "UIButton.hpp"
#include "MemoryFont.hpp"

class UI {
public:
    UI();
    ~UI();
    
    void draw();
    void setPosition(ofPoint point);
    
    void addSensor();
    void removeSensor();
    void addFilter();
    void removeFilter();
    void addOscSender();
    void removeOscSender();
    std::function<void()> onSaveCallback;
    
    void checkIfMouseOver(UIButton & button, ofPoint mousePoint);
    void checkIfMouseClicked(UIButton & button, ofPoint mousePoint);
    void checkIfMouseReleased(UIButton & button);

    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
    
    UIButton saveButton;
    UIButton addSensorButton;
    UIButton removeSensorButton;
    UIButton addFilterButton;
    UIButton removeFilterButton;
    UIButton addOscSenderButton;
    UIButton removeOscSenderButton;
    
    ofParameter<int> numberSensors;
    ofParameter<int> numberFilters;
    ofParameter<int> numberOscSenders;
    
    ofPoint position;
    
    MemoryFont uiFont, titleFont;
    
    float size, offset;
    

};

#endif /* UI_hpp */
