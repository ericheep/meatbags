//
//  Blob.hpp
//

#ifndef Blob_hpp
#define Blob_hpp

#include "ofMain.h"
#include <stdio.h>

class Blob {
public:
    Blob(vector<ofPoint> points);
    
    ofPoint centroid;
    ofRectangle bounds;
private:
    void calcCentroid(vector<ofPoint>& points);
    void calcBounds(vector<ofPoint>& points);
};

#endif /* Blob_hpp */
