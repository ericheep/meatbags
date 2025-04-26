//
//  Viewer.hpp
//

#ifndef Viewer_hpp
#define Viewer_hpp

#include "Blob.hpp"
#include "Bounds.hpp"
#include "Sensors.hpp"
#include "ofMain.h"
#include "Space.h"
#include <stdio.h>

class Viewer {
public:
    Viewer();
    
    void drawGrid();
    void drawDraggablePoints(Bounds& bounds);
    void drawBounds(Bounds& bounds);
    void drawSensors(Sensors& sensors);
    void drawConnections(Sensors& sensors);
    void drawCoordinates(vector<ofPoint>& coordinates, ofColor color);
    void drawSensor(Hokuyo* hokuyo);
    void drawBlobs(vector<Blob>& blobs);
    
    void setSpace(Space & space);
    
    Space space;
    Sensors sensor;
    Bounds bounds;
    float scale;
    
    ofTrueTypeFont blobFont, sensorFont;
    vector<ofColor> sensorColors;
};

#endif /* Viewer_hpp */
