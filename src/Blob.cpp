//
//  Blob.cpp
//

#include "Blob.hpp"

Blob::Blob() {
    matched = false;
}

Blob::Blob(vector<ofPoint> coordinates, vector<int> intensities) {
    calculateCentroid(coordinates);
    calculateBounds(coordinates);
    calculateIntensity(intensities);
    distanceFromSensor = centroid.length();

    matched = false;
}

void Blob::become(Blob _blob) {
    centroid = _blob.centroid;
    center = _blob.center;
    intensity = _blob.intensity;
    bounds = _blob.bounds;
    distanceFromSensor = _blob.distanceFromSensor;
}

void Blob::setIndex(int _index) {
    index = _index;
}

void Blob::setMatched(bool _matched) {
    matched = _matched;
}

bool Blob::isMatched() {
    return matched;
}

void Blob::setPotentialMatch(int _potentialMatchIndex, float _score) {
    potentialMatchIndex = _potentialMatchIndex;
    potentialMatchScore = _score;
}

void Blob::calculateCentroid(vector<ofPoint>& coordinates) {
    float totalX = 0.0;
    float totalY = 0.0;
    
    for (int i = 0; i < coordinates.size(); i++) {
        totalX += coordinates[i].x;
        totalY += coordinates[i].y;
    }
    
    centroid = ofPoint(totalX / coordinates.size(), totalY / coordinates.size());
}

void Blob::calculateBounds(vector<ofPoint>& coordinates) {
    float minX = 10000;
    float maxX = -10000;
    float minY = 10000;
    float maxY = -10000;
    
    for (int i = 0; i < coordinates.size(); i++) {
        float x = coordinates[i].x;
        float y = coordinates[i].y;

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
    center = ofPoint(rX + rW * 0.5, rY + rH * 0.5);
}

void Blob::calculateIntensity(vector<int>& intensities) {
    int sum = 0;
    for (int i = 0; i < intensities.size(); i++) {
        sum += intensities[i];
    }
    intensity = (float)sum / intensities.size();
}
