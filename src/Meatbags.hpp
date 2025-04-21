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
#include "Bounds.hpp"

class Meatbags {
public:
    Meatbags();
        
    void update();
    void updateBlobs();
    void updateDraggablePoints();
    void updateBounds();
    
    void filterCoordinates();
    void calculateBlobs();
    
    void clusterBlobs();
    void matchBlobs();
    void addBlobs();
    void removeBlobs();
    void getBlobs(vector<Blob> &blob);

    int findFreeBlobIndex();
    float compareBlobs(Blob newBlob, Blob oldBlob);
        
    void setBounds(float x1, float y1, float x2, float y2);
    void setBlobPersistence(float blobPersistence);
    void setAreaSize(float areaSize);
    void setMirrorX(bool mirrorX);
    void setCanvasSize(float width, float height);
    void setEpsilon(float epsilon);
    void setMinPoints(int minPoints);
    float pointDistance(ofPoint a, ofPoint b);
    
    void saveToFile(const string& path);
    void loadFile(const string& path);
    
    void setFont(ofTrueTypeFont globalFont);

    vector<Blob> newBlobs, oldBlobs;
    vector<ofPoint> coordinates;
    vector<ofPoint> filteredCoordinates;
    vector<int> intensities;
    vector<int> filteredIntensities;

    float boundsX1, boundsX2, boundsY1, boundsY2;
    float width, height;
    float blobPersistence;
    
    // dbscan params
    float epsilon;
    int minPoints;
    
    float lastUpdateTime;
    float timeBetweenUpdates;
    int numberFilteredCoordinates;
};

#endif /* Viz_hpp */
