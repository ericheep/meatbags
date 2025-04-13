//
//  Blob.cpp
//

#include "Blob.hpp"

Blob::Blob(vector<ofPoint> points) {
    calcCentroid(points);
    calcBounds(points);
}

void Blob::calcCentroid(vector<ofPoint>& points) {
    float totalX = 0.0;
    float totalY = 0.0;
    
    for (int i = 0; i < points.size(); i++) {
        totalX += points[i].x;
        totalY += points[i].y;
    }
    
    centroid = ofPoint(totalX / points.size(), totalY / points.size());
}

void Blob::calcBounds(vector<ofPoint>& points) {
    float minX = 10000;
    float maxX = -10000;
    float minY = 10000;
    float maxY = -10000;
    
    for (int i = 0; i < points.size(); i++) {
        float x = points[i].x;
        float y = points[i].y;

        if (x > maxX) maxX = x;
        if (x < minX) minX = x;
        if (y > maxY) maxY = y;
        if (y < minY) minY = y;
    }
    
    float rX = minX;
    float rY = minY;
    float rW = fabs(maxX - minX);
    float rH = fabs(maxY - minY);
        
    bounds = ofRectangle(rX, rY, rW, rH);
}
