//
//  Bounds.hpp
//

#ifndef Bounds_hpp
#define Bounds_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Space.h"
#include "DraggablePoint.hpp"

class Bounds {
public:
    Bounds();
    ~Bounds();
    
    Bounds(int numberPoints);
        
    void update();
    void setPoint(int index, ofVec2f point);
    void setAreaSize(float areaSize);
    void setCanvasSize(float width, float height);
    void setOrigin(ofPoint origin);
    void setSpace(Space & space);
    ofPoint convertCoordinateToScreenPoint(ofPoint coordinate);
    ofPoint convertScreenPointToCoordinate(ofPoint screenPoint);

    vector<ofParameter<ofVec2f>> points;
    vector<DraggablePoint> positions;

    float mouseBoxHalfSize, mouseBoxSize;
    float scale;
    int numberPoints;
    
    Space space;
    ofPolyline polyline;
    int selectedDraggablePointIndex, highlightedDraggablePointIndex;
    
protected:
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
};

#endif /* Bounds_hpp */
