//
//  Viewer.cpp
//

#include "Viewer.hpp"

Viewer::Viewer() {
    width = 0;
    height = 0;
    scale = 0.0;
    areaSize = 0;
    origin = ofPoint(ofGetWidth() / 2.0, 25);
    ofSetCircleResolution(3);
    
    blobFont.setGlobalDpi(72);
    blobFont.load(ofToDataPath("Hack-Bold.ttf"), 14);
}

void Viewer::setAreaSize(float _areaSize) {
    areaSize = _areaSize;
    scale = width / (areaSize * 1000);
}

void Viewer::setCanvasSize(float _width, float _height) {
    width = _width;
    height = _height;
    
    origin = ofPoint(width / 2.0, 25);
    setAreaSize(areaSize);
}

void Viewer::setBounds(Bounds& _bounds) {
    bounds = _bounds;
}

void Viewer::drawGrid() {
    ofSetColor(ofColor::grey);
    float crossHalfLength = scale * 25;

    for (int i = 0; i < (int) areaSize + 4; i++) {
        int gridLineIndex = i + -( (int) areaSize + 4) / 2;
        
        for (int j = 0; j < (int) areaSize + 6; j++) {
            float x = gridLineIndex * 1000.0 * scale + origin.x;
            float y = j * 1000.0 * scale + origin.y;
        
            for (int k = 0; k < 3; k++) {
                float ex = ofMap(k, 0, 3 - 1, x - scale * 333, x + scale * 333);
                float ey = ofMap(k, 0, 3 - 1, y - scale * 333, y + scale * 333);
                
                ofDrawLine(ex - crossHalfLength, y, ex + crossHalfLength, y);
                ofDrawLine(x, ey - crossHalfLength, x, ey + crossHalfLength);
            }
        }
    }
}

void Viewer::drawBlobs(vector<Blob>& blobs) {
    for (auto blob : blobs) {
        ofRectangle blobBox = blob.bounds;
        blobBox.setX(blobBox.getX() * scale + origin.x);
        blobBox.setY(blobBox.getY() * scale + origin.y);
        blobBox.setWidth(blobBox.getWidth() * scale);
        blobBox.setHeight(blobBox.getHeight() * scale);
        
        ofNoFill();
        ofSetColor(0, 0, 255);
        ofDrawRectangle(blobBox);
        
        float centroidX = blob.centroid.x * scale + origin.x;
        float centroidY = blob.centroid.y * scale + origin.y;
        
        ofFill();
        ofSetColor(255, 0, 0);
        ofDrawEllipse(centroidX, centroidY, 9, 9);

        std::stringstream index;
        index << blob.index;
        std::stringstream x;
        x << setprecision(2) << blob.centroid.x / 1000.0;
        std::stringstream y;
        y << setprecision(2) << blob.centroid.y / 1000.0;
       
        ofSetColor(255, 0, 0);
        string fontString =
        index.str() + "\n" +
        "x: " + x.str() + "\n" +
        "y: " + y.str() + "\n" +
        "points: " + to_string(blob.numberPoints);
        
        blobFont.drawString(fontString, centroidX + 15, centroidY - 15);
    }
}

void Viewer::drawBounds() {
    ofNoFill();
    ofSetColor(ofColor::magenta);
    float bx = bounds.x1 * 1000 * scale + origin.x;
    float by = bounds.y1 * 1000 * scale + origin.y;
    float bw = fabs((bounds.x2 - bounds.x1) * 1000) * scale;
    float bh = fabs((bounds.y2 - bounds.y1) * 1000) * scale;
    ofDrawRectangle(bx, by, bw, bh);
}

void Viewer::drawCoordinates(vector<ofPoint>& coordinates, ofColor color) {
    for (int i = 0; i < coordinates.size(); i++) {
        ofColor pointColor;

        float x = coordinates[i].x;
        float y = coordinates[i].y;
        
        if (x == 0 && y == 0) break;

        if (x < bounds.x2 * 1000
            && x > bounds.x1 * 1000
            && y > bounds.y1 * 1000
            && y < bounds.y2 * 1000)
        {
            pointColor.set(color.r, color.g, color.b, 255);
        } else {
            pointColor.set(color.r, color.g, color.b, 100);
        }
        
        x *= scale;
        y *= scale;
        
        float x2 = x + origin.x;
        float y2 = y + origin.y;
        
        ofFill();
        ofSetColor(pointColor);
        ofDrawEllipse(x2, y2, 3, 3);
    }
}

void Viewer::drawSensor(ofPoint position, float rotation, ofColor color) {
    position *= scale;
    position += origin;
    float x = position.x;
    float y = position.y;
    
    float size = 16;
    float halfSize = size * 0.5;
    
    ofSetColor(color);
    ofFill();
    ofPushMatrix();
    ofTranslate(x, y);
    ofRotateRad(rotation);
    ofDrawLine(0, halfSize, 0, size + halfSize);
    ofDrawRectangle(-halfSize, -halfSize, size, size);
    ofPopMatrix();
}

void Viewer::drawDraggablePoints() {
    for (int i = 0; i < 4; i++) {
        float x1 = bounds.draggablePoints[i].x - bounds.mouseBoxHalfSize;
        float y1 = bounds.draggablePoints[i].y - bounds.mouseBoxHalfSize;
        float x2 = bounds.mouseBoxSize;
        float y2 = bounds.mouseBoxSize;
        
        ofNoFill();
        
        if (bounds.highlightedDraggablePointIndex == i) ofFill();
        
        ofSetColor(ofColor::magenta);
        ofDrawRectangle(x1, y1, x2, y2);
    }
}
