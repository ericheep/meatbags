//
//  Blob.hpp
//

#ifndef Blob_hpp
#define Blob_hpp

#include "ofMain.h"
#include <stdio.h>

class Blob {
public:
    Blob();
    Blob(vector<ofPoint> coordinates, vector<int> intensities);
    void setMatched(bool matched);
    bool isMatched();
    void setIndex(int index);
    void setPotentialMatch(int index, float score);
    void become(Blob blob);

    ofPoint centroid, center;
    ofRectangle bounds;
    float intensity;
    
    int potentialMatchIndex;
    float potentialMatchScore;
    
    bool matched;
    
    int index;
private:
    void calculateCentroid(vector<ofPoint>& coordinates);
    void calculateBounds(vector<ofPoint>& coordinates);
    void calculateIntensity(vector<int>& intensity);
};

#endif /* Blob_hpp */
