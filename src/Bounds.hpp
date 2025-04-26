//
//  Bounds.hpp
//

#ifndef Bounds_hpp
#define Bounds_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Space.h"

class Bounds {
public:
    Bounds();
    
    void updateDraggablePoints();
    void updateBounds();
    void setBounds(float x1, float x2, float y1, float y2);
    void setAreaSize(float areaSize);
    void setCanvasSize(float width, float height);
    void setOrigin(ofPoint origin);
    void setSpace(Space & space);
    
    float x1, x2, y1, y2;
    float mouseBoxHalfSize, mouseBoxSize;
    float scale;
    
    Space space;
    vector <ofPoint> draggablePoints;
    int selectedDraggablePointIndex, highlightedDraggablePointIndex;
    
protected:
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
};

#endif /* Bounds_hpp */
