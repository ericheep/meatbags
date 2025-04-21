//
//  Bounds.hpp
//

#ifndef Bounds_hpp
#define Bounds_hpp

#include <stdio.h>
#include "ofMain.h"

class Bounds {
public:
    Bounds();
    
    void updateDraggablePoints();
    void updateBounds();
    void setBounds(float x1, float x2, float y1, float y2);
    void setAreaSize(float areaSize);
    void setCanvasSize(float width, float height);
    
    float x1, x2, y1, y2;
    float mouseBoxHalfSize, mouseBoxSize;
    float scale, width, height, areaSize;
    
    ofPoint origin;
    vector <ofPoint> draggablePoints;
    int selectedDraggablePointIndex, highlightedDraggablePointIndex;

protected:
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
};

#endif /* Bounds_hpp */
