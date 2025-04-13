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
    
    void polarToCartesian();
    void filterCoordinates();
    void calculateBlobs();
    void compareBlobs();
    
    void setScale(float scale);
    void setSize(float width, float height);
    void setScanningArea(float x1, float x2, float y1, float y2);
    void setFilterBounds(float x1, float x2, float y1, float y2);
    void setEpsilon(float epsilon);
    void setMinPoints(int minPoints);
    float pointDistance(ofPoint a, ofPoint b);

    vector<Blob> currentBlobs, blobs;
    vector<ofPoint> polarCoordinates;
    vector<ofPoint> cartesianCoordinates;
    vector<ofPoint> filteredCoordinates;
    vector<int> intensities;
    vector<int> filteredIntensities;


private:
    float width, height;
    float areaX1, areaX2, areaY1, areaY2;
    float boundsX1, boundsX2, boundsY1, boundsY2, scale;
    
    // dbscan params
    float epsilon;
    int minPoints;
    
    ofPoint puckPosition;
};

#endif /* Viz_hpp */
