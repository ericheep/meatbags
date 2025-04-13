//
//  Blob.hpp
//

#ifndef Blob_hpp
#define Blob_hpp

#include "ofMain.h"
#include <stdio.h>

class Blob {
public:
    Blob(vector<ofPoint> coordinates, vector<int> intensities);
    void setIndex(int index);

    ofPoint centroid, center;
    ofRectangle bounds;
    float intensity;
    
    int index;
private:
    void calculateCentroid(vector<ofPoint>& coordinates);
    void calculateBounds(vector<ofPoint>& coordinates);
    void calculateIntensity(vector<int>& intensity);
};

#endif /* Blob_hpp */
