//
//  Viz.hpp
//

#ifndef Viz_hpp
#define Viz_hpp

#include "ofMain.h"
#include <stdio.h>
#include "dbscan.hpp"
#include "Blob.hpp"

class Viz {
public:
    Viz();
    
    void draw();
    void update();
    void filter();
    void polarToCartesian();
    void getBlobs();
    
    void setScale(float scale);
    void setSize(float width, float height);
    void setScanningArea(float x1, float x2, float y1, float y2);
    void setFilterBounds(float x1, float x2, float y1, float y2);
    void setEpsilon(float epsilon);
    void setMinPoints(int minPoints);

    vector<Blob> blobs;
    vector<ofPoint> polarCoordinates;
    vector<ofPoint> cartesianCoordinates;
    vector<ofPoint> filteredCoordinates;
    vector<int> intensities;

private:
    float width, height;
    float areaX1, areaX2, areaY1, areaY2;
    float boundsX1, boundsX2, boundsY1, boundsY2, scale;
    float epsilon;
    int minPoints;
    
    ofPoint puckPosition;
};

#endif /* Viz_hpp */
