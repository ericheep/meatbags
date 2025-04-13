//
//  Viz.cpp
//

#include "Viz.hpp"

Viz::Viz() {
    polarCoordinates.resize(1024);
    cartesianCoordinates.resize(1024);
    intensities.resize(1024);
    
    epsilon = 100.0;
    minPoints = 5;
}

void Viz::update() {
    puckPosition = ofPoint(width / 2.0, 50);
    polarToCartesian();
    filter();
    getBlobs();
}

void Viz::getBlobs() {
    if (filteredCoordinates.size() == 0) return;
        
    vector<struct point2> points;
    for (int i = 0; i < filteredCoordinates.size(); i++) {
        struct point2 point;
        point.x = filteredCoordinates[i].x;
        point.y = filteredCoordinates[i].y;
        points.push_back(point);
    }
    
    auto clusters = dbscan(points, epsilon, minPoints);
    
    blobs.clear();
    for(auto& cluster: clusters) {
        vector<ofPoint> points;
        for (int i = 0; i < cluster.size(); i++) {
            int index = cluster[i];
            points.push_back(filteredCoordinates[index]);;
        }
        blobs.push_back(Blob(points));
    }
}

void Viz::polarToCartesian() {
    for (int i = 0; i < polarCoordinates.size(); i++) {
        float theta = polarCoordinates[i].x;
        float radius = polarCoordinates[i].y;

        // theta = ofMap(theta, 0, TWO_PI, TWO_PI, 0);
        
        float x = cos(theta) * radius;
        float y = sin(theta) * radius;
        
        cartesianCoordinates[i].x = x;
        cartesianCoordinates[i].y = y;
    }
}

void Viz::filter() {
    filteredCoordinates.clear();
    
    for (int i = 0; i < cartesianCoordinates.size(); i++) {
        float x = cartesianCoordinates[i].x;
        float y = cartesianCoordinates[i].y;
        
        if (x != 0 && y != 0) {
            if (x < boundsX2 && x > boundsX1 && y > boundsY1 && y < boundsY2) {
                filteredCoordinates.push_back(ofPoint(x, y));
            }
        }
    }
}

void Viz::draw() {
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

        std::stringstream strm;
        strm << setprecision(3) << "x: " << centroidX / 1000.0;
        strm << setprecision(3) << " y: " << centroidY / 1000.0;
        
        ofDrawBitmapString(strm.str(), centroidX + 15, centroidY - 4);
        
        float distance = sqrt(pow(blob.centroid.x, 2) + pow(blob.centroid.y, 2)) / 1000.0;
        
        std::stringstream distanceStrm;
        distanceStrm << setprecision(3) << "distance: " << distance;
        distanceStrm << setprecision(3) << "m " << distance * 3.28084 << "ft";

        ofDrawBitmapString(distanceStrm.str(), centroidX + 15, centroidY + 11);
    }
}

void Viz::setSize(float _width, float _height) {
    width = _width;
    height = _height;
}

void Viz::setScale(float _scale) {
    scale = _scale;
}

void Viz::setScanningArea(float _areaX1, float _areaX2, float _areaY1, float _areaY2) {
    // meters to millimeers
    areaX1 = _areaX1 * 1000;
    areaX2 = _areaX2 * 1000;
    areaY1 = _areaY1 * 1000;
    areaY2 = _areaY2 * 1000;
}

void Viz::setFilterBounds(float _boundsX1, float _boundsX2, float _boundsY1, float _boundsY2) {
    // meters to millimeers
    boundsX1 = _boundsX1 * 1000;
    boundsX2 = _boundsX2 * 1000;
    boundsY1 = _boundsY1 * 1000;
    boundsY2 = _boundsY2 * 1000;
}

void Viz::setEpsilon(float _epsilon) {
    epsilon = _epsilon;
}

void Viz::setMinPoints(int _minPoints) {
    minPoints = _minPoints;
}
