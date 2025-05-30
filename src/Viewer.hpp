//
//  Viewer.hpp
//

#ifndef Viewer_hpp
#define Viewer_hpp

#include "Blob.hpp"
#include "Filter.hpp"
#include "Sensors.hpp"
#include "Filters.hpp"
#include "ofMain.h"
#include "Space.h"
#include "MemoryFont.hpp"
#include <stdio.h>

class Viewer {
public:
    Viewer();
    ~Viewer();
    
    void draw(vector<Blob> & blobs, Filters & filters, Sensors & sensors);
    void drawGrid();
    void drawDraggablePoints(Filter & bounds);
    void drawDraggablePoints(Filter * filter);

    void drawFilter(Filter * filter);
    void drawFilters(Filters & filters);
    void drawSensors(Sensors & sensors, Filters & filters);
    void drawConnections(Sensors & sensors);
    void drawCoordinates(vector<ofPoint> & coordinates, ofColor color, Filters & filters);
    void drawCursorCoordinate();
    
    void drawSensor(Hokuyo * hokuyo);
    void drawBlobs(vector<Blob> & blobs);
    void setSpace(Space & space);
    void setTranslation(ofPoint translation);
    void setCursorString(ofPoint mousePoint);
    
    bool checkWithinBounds(float x, float y, Filters & filters);
    
    Space space;
    Sensors sensor;
    float scale;
    ofPoint translation;
    
    MemoryFont blobFont, sensorFont, filterFont, cursorFont;
    string cursorString;
    vector<ofColor> sensorColors;
    
protected:
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
};

#endif /* Viewer_hpp */
