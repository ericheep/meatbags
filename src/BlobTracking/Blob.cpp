//
//  Blob.cpp
//

#include "Blob.hpp"

Blob::Blob() {
    matched = false;
    alive = true;
    lifetime = 0;
    numberPoints = 0;
    whichMeatbag = 0;
}

Blob::Blob(vector<ofPoint> coordinates, vector<int> intensities, float blobPersistence, int _numberPoints) {
    calculateCentroid(coordinates);
    calculateBounds(coordinates);
    calculateIntensity(intensities);
    
    distanceFromSensor = centroid.length();
    lifetimeLength = blobPersistence;
    matched = false;
    alive = true;
    lifetime = 0;
    numberPoints = _numberPoints;
}

void Blob::updateLifetime(float secondsLived) {
    lifetime += secondsLived;
    if (lifetime > lifetimeLength) {
       alive = false;
    }
}

bool Blob::isAlive() {
    return alive;
}

void Blob::become(Blob &_blob) {
    centroid = _blob.centroid;
    center = _blob.center;
    intensity = _blob.intensity;
    bounds = _blob.bounds;
    distanceFromSensor = _blob.distanceFromSensor;
    numberPoints = _blob.numberPoints;
    lifetimeLength = _blob.lifetimeLength;
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

void Blob::calculateCentroid(vector<ofPoint> &coordinates) {
    float totalX = 0.0;
    float totalY = 0.0;
    
    for (const auto& point : coordinates) {
        totalX += point.x;
        totalY += point.y;
    }
    
    centroid = ofPoint(totalX / coordinates.size(), totalY / coordinates.size());
}

void Blob::calculateBounds(vector<ofPoint> &coordinates) {
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    
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

void Blob::calculateIntensity(vector<int> &intensities) {
    int sum = 0;
    for (int i = 0; i < intensities.size(); i++) {
        sum += intensities[i];
    }
    intensity = (float)sum / intensities.size();
}
