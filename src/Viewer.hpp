//
//  Viewer.hpp
//

#ifndef Viewer_hpp
#define Viewer_hpp

#include "Blob.hpp"
#include "Bounds.hpp"
#include "ofMain.h"
#include <stdio.h>

class Viewer {
public:
    Viewer();
    
    void drawGrid();
    void drawDraggablePoints();
    void drawBounds();
    void drawCoordinates(vector<ofPoint>& coordinates, ofColor color);
    void drawSensor(ofPoint position, float rotation, ofColor color);
    void drawBlobs(vector<Blob>& blobs);

    void setBounds(Bounds& bounds);
    void setCanvasSize(float width, float height);
    void setAreaSize(float areaSize);
    void setSensorColors(vector<ofColor> colors);
    
    Bounds bounds;
    ofPoint origin;
    float areaSize, scale, width, height;
    float mouseBoxSize, mouseBoxHalfSize;
    
    ofTrueTypeFont blobFont;
    vector<ofColor> sensorColors;
};

#endif /* Viewer_hpp */
