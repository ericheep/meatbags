//
//  Meatbags.hpp
//  https://www.mit.edu/people/dpolicar/writing/prose/text/thinkingMeat.html
//

#ifndef Viz_hpp
#define Viz_hpp

#include "ofMain.h"
#include <stdio.h>
#include <algorithm>
#include "dbscan.hpp"
#include "Blob.hpp"

class Meatbags {
public:
    Meatbags();
        
    void update();
    void updateBlobs();
    void updateDraggablePoints();
    void updateBounds();
    
    void filterCoordinates();
    
    void clusterBlobs();
    void matchBlobs();
    void addBlobs();
    void renewBlobs();
    void getBlobs(vector<Blob> &blob);

    int findFreeBlobIndex();
    float compareBlobs(Blob newBlob, Blob oldBlob);
    
    void setMirrorX(bool mirrorX);
    float pointDistance(ofPoint a, ofPoint b);
    
    void setFont(ofTrueTypeFont globalFont);
    void setBlobPersistence(float & blobPersistence);
    void setMaxCoordinateSize(int maxSize);

    vector<Blob> newBlobs, oldBlobs;
    vector<ofPoint> coordinates;
    vector<int> intensities;
    
    ofParameter<float> blobPersistence;
    ofParameter<float> epsilon;
    ofParameter<int> minPoints;
    
    float lastFrameTime;
    int numberCoordinates;
};

#endif /* Viz_hpp */
