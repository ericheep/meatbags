//
//  Blob.hpp
//

#ifndef Blob_hpp
#define Blob_hpp

#include "ofMain.h"
#include <stdio.h>
#include <limits>

class Blob {
public:
    Blob();
    Blob(vector<ofPoint> coordinates, vector<int> intensities, float blobPersistence, int numberPoints);
    void updateLifetime(float secondsLived);
    void setMatched(bool matched);
    bool isMatched();
    bool isAlive();
    void setIndex(int index);
    void setPotentialMatch(int index, float score);
    void become(Blob &blob);

    ofPoint centroid, center;
    ofRectangle bounds;
    int numberPoints;
    float intensity;
    float distanceFromSensor;
    
    int potentialMatchIndex;
    float potentialMatchScore;
    float lifetime, lifetimeLength;
    
    bool matched, alive;
    
    int index, whichMeatbag;
private:
    void calculateCentroid(vector<ofPoint> &coordinates);
    void calculateBounds(vector<ofPoint> &coordinates);
    void calculateIntensity(vector<int> &intensity);
};

#endif /* Blob_hpp */
