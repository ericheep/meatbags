//
//  Viewer.hpp
//

#ifndef Viewer_hpp
#define Viewer_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxOpenCv.h"

#include "Blob.hpp"
#include "Filter.hpp"
#include "Sensor.hpp"
#include "Space.h"
#include "LidarPoint.h"
#include "MemoryFont.hpp"

class Viewer {
public:
    Viewer();
    ~Viewer();
    
    void draw(const vector<Blob>& blobs, const vector<Filter*>& filters_, const vector<Sensor*>& sensors_);
    void drawGrid();
    void drawDraggablePoints(const Filter& bounds);
    void drawDraggablePoints(const Filter* filter);
    void drawCoordinates(vector<LidarPoint>& lidarPoints, int numberLidarPoints);

    void drawFilter(Filter* filter);
    void drawFilters(const vector<Filter*>& filters);
    void drawSensors(const vector<Sensor*>& sensors);
    void drawConnections(const vector<Sensor*>& sensors);
    void drawCursorCoordinate();
    void drawHelpText();
    void drawSaveNotification();
    
    void drawSensor(const Sensor* sensor);
    void drawBlobs(const vector<Blob>& blobs);
    void setSpace(const Space& space);
    void setTranslation(ofPoint translation);
    void setCursorString(const ofPoint& mousePoint);
    void onMouseMoved(ofMouseEventArgs& mouseArgs);
    void onMouseDragged(ofMouseEventArgs& mouseArgs);
    
    ofMesh mesh, circleMesh;
    void initializeCircleMeshes();
    void initializeTrianglesMesh(const vector<LidarPoint>& lidarPoints, int numberLidarPoints);

    Space space;
    float scale;
    ofPoint translation;
    
    MemoryFont blobFont, sensorFont, filterFont, cursorFont;
    MemoryFont titleFont, helpFont, saveFont;
    
    int circleResolution;
    string cursorString;
    vector<ofColor> sensorColors;
    string version = "0.6.1";
};

#endif /* Viewer_hpp */
