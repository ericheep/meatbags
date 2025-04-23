//
//  MouseUI.hpp
//


#ifndef MouseUI_hpp
#define MouseUI_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Bounds.hpp"

class MouseUI {
public:
    MouseUI();
    
    void update();
    void setBounds(Bounds & bounds);
    
    vector <ofPoint> draggableBoundsPositions;
    vector <ofPoint> draggableSensorPositions;

    Bounds bounds;
    
    float mouseBoxHalfSize;
    int selectedDraggablePointIndex;
protected:
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
};

#endif /* MouseUI_hpp */
