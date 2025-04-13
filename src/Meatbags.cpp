//
//  Meatbags.cpp
//

#include "Meatbags.hpp"

Meatbags::Meatbags() {
    polarCoordinates.resize(1024);
    cartesianCoordinates.resize(1024);
    intensities.resize(1024);
    
    epsilon = 100.0;
    minPoints = 5;
}

void Meatbags::update() {
    puckPosition = ofPoint(width / 2.0, 50);
    polarToCartesian();
    filterCoordinates();
    calculateBlobs();
}

void Meatbags::polarToCartesian() {
    for (int i = 0; i < polarCoordinates.size(); i++) {
        float theta = polarCoordinates[i].x;
        float radius = polarCoordinates[i].y;
        
        float x = cos(theta) * radius;
        float y = sin(theta) * radius;
        
        cartesianCoordinates[i].x = x;
        cartesianCoordinates[i].y = y;
    }
}

void Meatbags::filterCoordinates() {
    filteredCoordinates.clear();
    
    for (int i = 0; i < cartesianCoordinates.size(); i++) {
        float x = cartesianCoordinates[i].x;
        float y = cartesianCoordinates[i].y;
        
        if (x != 0 && y != 0) {
            if (x < boundsX2 && x > boundsX1 && y > boundsY1 && y < boundsY2) {
                filteredCoordinates.push_back(ofPoint(x, y));
                filteredIntensities.push_back(intensities[i]);
            }
        }
    }
}

float Meatbags::pointDistance(ofPoint a, ofPoint b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

void Meatbags::calculateBlobs() {
    if (filteredCoordinates.size() == 0) return;
        
    vector<struct point2> points;
    for (int i = 0; i < filteredCoordinates.size(); i++) {
        struct point2 point;
        point.x = filteredCoordinates[i].x;
        point.y = filteredCoordinates[i].y;
        points.push_back(point);
    }
    
    auto clusters = dbscan(points, epsilon, minPoints);
    
    currentBlobs.clear();
    
    int index = 0;
    for(auto& cluster: clusters) {
        vector<ofPoint> coordinates;
        vector<int> intensities;
        for (int i = 0; i < cluster.size(); i++) {
            int index = cluster[i];
            coordinates.push_back(filteredCoordinates[index]);;
            intensities.push_back(filteredIntensities[index]);
        }
        Blob blob = Blob(coordinates, intensities);
        blob.index = index;
        currentBlobs.push_back(blob);
        
        index++;
    }
    
    blobs.clear();
    if (blobs.size() == 0) {
        for (auto currentBlob : currentBlobs) {
            blobs.push_back(currentBlob);
        }
    }
}

void Meatbags::compareBlobs(Blob newBLob, Blob oldBLob) {
    if (currentBlobs.size() > blobs.size()) {
        for (auto currentBlob : currentBlobs) {
            float lowestDistance = 10000;
            
            for (auto blob : blobs) {
                float distance = pointDistance(blob.center, currentBlob.center);
                
                if (distance < lowestDistance) {
                    lowestDistance = distance;
                }
            }
        }
    }
    
    if (currentBlobs.size() < blobs.size()) {
        for (auto currentBlob : currentBlobs) {
            blobs.push_back(currentBlob);
            
            for (auto blob : blobs) {
                
            }
        }
    }
}

void Meatbags::draw() {
    ofNoFill();
    ofSetColor(255);
    ofDrawRectangle(0, 0, width, height);
    
    float scanningWidth = fabs(areaX2 - areaX1);
    float scanningHeight = fabs(areaY2 - areaY1);
    float scaleWidth = width / scanningWidth;
    float scaleHeight = height / scanningHeight;
    
    for (int i = 0; i < cartesianCoordinates.size(); i++) {
        float x = cartesianCoordinates[i].x;
        float y = cartesianCoordinates[i].y;
        
        if (x < boundsX2 && x > boundsX1 && y > boundsY1 && y < boundsY2) {
            ofSetColor(255);
        } else {
            ofSetColor(100);
        }
        
        x *= scaleWidth;
        y *= scaleHeight;

        float x2 = x + puckPosition.x;
        float y2 = y + puckPosition.y;
        
        ofFill();
        ofDrawEllipse(x2, y2, 3, 3);
    }
    
    ofDrawEllipse(puckPosition.x, puckPosition.y, 10, 10);
    ofNoFill();
    ofSetColor(ofColor::magenta);
    float bx = boundsX1 * scaleWidth + puckPosition.x;
    float by = boundsY1 * scaleHeight + puckPosition.y;
    float bw = fabs(boundsX2 - boundsX1) * scaleWidth;
    float bh = fabs(boundsY2 - boundsY1) * scaleHeight;
    ofDrawRectangle(bx, by, bw, bh);
    
    for (auto blob : blobs) {
        ofRectangle blobBox = blob.bounds;
        blobBox.setX(blobBox.getX() * scaleWidth + puckPosition.x);
        blobBox.setY(blobBox.getY() * scaleHeight + puckPosition.y);
        blobBox.setWidth(blobBox.getWidth() * scaleWidth);
        blobBox.setHeight(blobBox.getHeight() * scaleHeight);
        
        ofNoFill();
        ofSetColor(0, 0, 255);
        ofDrawRectangle(blobBox);
        
        float centroidX = blob.centroid.x * scaleWidth + puckPosition.x;
        float centroidY = blob.centroid.y * scaleHeight + puckPosition.y;
        
        ofFill();
        ofSetColor(255, 0, 0);
        ofDrawEllipse(centroidX, centroidY, 9, 9);

        std::stringstream centroidStrm;
        centroidStrm << setprecision(3) << "x: " << centroidX / 1000.0;
        centroidStrm << setprecision(3) << " y: " << centroidY / 1000.0;
        ofDrawBitmapString(centroidStrm.str(), centroidX + 15, centroidY - 4);
        
        float distance = sqrt(pow(blob.centroid.x, 2) + pow(blob.centroid.y, 2)) / 1000.0;
        
        std::stringstream distanceStrm;
        distanceStrm << setprecision(3) << "distance: " << distance << "m";
        ofDrawBitmapString(distanceStrm.str(), centroidX + 15, centroidY + 11);
        
        std::stringstream intensityStrm;
        intensityStrm << setprecision(5) << "intensity: " << blob.intensity;
        ofDrawBitmapString(intensityStrm.str(), centroidX + 15, centroidY + 26);

    }
}

void Meatbags::setSize(float _width, float _height) {
    width = _width;
    height = _height;
}

void Meatbags::setScale(float _scale) {
    scale = _scale;
}

void Meatbags::setScanningArea(float _areaX1, float _areaX2, float _areaY1, float _areaY2) {
    // meters to millimeers
    areaX1 = _areaX1 * 1000;
    areaX2 = _areaX2 * 1000;
    areaY1 = _areaY1 * 1000;
    areaY2 = _areaY2 * 1000;
}

void Meatbags::setFilterBounds(float _boundsX1, float _boundsX2, float _boundsY1, float _boundsY2) {
    // meters to millimeers
    boundsX1 = _boundsX1 * 1000;
    boundsX2 = _boundsX2 * 1000;
    boundsY1 = _boundsY1 * 1000;
    boundsY2 = _boundsY2 * 1000;
}

void Meatbags::setEpsilon(float _epsilon) {
    epsilon = _epsilon;
}

void Meatbags::setMinPoints(int _minPoints) {
    minPoints = _minPoints;
}
