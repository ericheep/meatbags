//
//  Viewer.cpp
//

#include "Viewer.hpp"

Viewer::Viewer() {
    scale = 0.0;
    ofSetCircleResolution(3);
    
    blobFont.setGlobalDpi(72);
    blobFont.load(ofToDataPath("Hack-Bold.ttf"), 14);
    
    sensorFont.load(ofToDataPath("Hack-Bold.ttf"), 12);

    
}

void Viewer::setSpace(Space & _space) {
    space = _space;
    scale = space.width / (space.areaSize * 1000);

}

void Viewer::drawGrid() {
    ofFill();
    ofSetCircleResolution(23);
    ofColor originColor = ofColor::darkSalmon;
    originColor.a = 110;
    ofSetColor(originColor);
    ofDrawCircle(space.origin, 6);
    
    ofSetColor(ofColor::grey);
    float crossHalfLength = scale * 25;

    for (int i = 0; i < (int) space.areaSize + 4; i++) {
        int gridLineIndex = i + -( (int) space.areaSize + 4) / 2;
        
        for (int j = 0; j < (int) space.areaSize + 6; j++) {
            float x = gridLineIndex * 1000.0 * scale + space.origin.x;
            float y = j * 1000.0 * scale + space.origin.y;
        
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
        blobBox.setX(blobBox.getX() * scale + space.origin.x);
        blobBox.setY(blobBox.getY() * scale + space.origin.y);
        blobBox.setWidth(blobBox.getWidth() * scale);
        blobBox.setHeight(blobBox.getHeight() * scale);
        
        ofNoFill();
        ofSetColor(0, 0, 255);
        ofDrawRectangle(blobBox);
        
        float centroidX = blob.centroid.x * scale + space.origin.x;
        float centroidY = blob.centroid.y * scale + space.origin.y;
        
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

void Viewer::drawBounds(Bounds& bounds) {
    ofNoFill();
    ofSetColor(ofColor::magenta);
    float bx = bounds.x1 * 1000 * scale + space.origin.x;
    float by = bounds.y1 * 1000 * scale + space.origin.y;
    float bw = fabs((bounds.x2 - bounds.x1) * 1000) * scale;
    float bh = fabs((bounds.y2 - bounds.y1) * 1000) * scale;
    ofDrawRectangle(bx, by, bw, bh);
    
    drawDraggablePoints(bounds);
}

void Viewer::drawCoordinates(vector<ofPoint>& coordinates, ofColor color) {
    for (auto& coordinate : coordinates) {
        ofColor pointColor;

        float x = coordinate.x;
        float y = coordinate.y;
        
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
        
        float x2 = x + space.origin.x;
        float y2 = y + space.origin.y;
        
        ofFill();
        ofSetColor(pointColor);
        ofDrawEllipse(x2, y2, 3, 3);
    }
}

void Viewer::drawSensors(Sensors& sensors) {
    for (auto& sensor : sensors.hokuyos) {
        drawCoordinates(sensor->coordinates, sensor->sensorColor);
        drawSensor(sensor);
        if (sensor->showSensorInformation) sensor->draw();
    }
    
    drawConnections(sensors);
}

void Viewer::drawConnections(Sensors& sensors) {
    int numberSensors = sensors.hokuyos.size();
    
    float connectionsBoxHeight = numberSensors * 20;
    float y = space.height - connectionsBoxHeight + 5;
    float x = 10;
    
    for (int i = 0; i < numberSensors; i++) {
        bool connected = sensors.hokuyos[i]->isConnected;
        string model = sensors.hokuyos[i]->model;

        ofFill();
        if (connected) {
            ofSetColor(0, 255, 0, 130);
        } else {
            ofSetColor(255, 0, 0, 130);
        }
        string sensorString = "Sensor " + to_string(i + 1) + ": " + model;
        ofDrawRectangle(x, y + i * 20 - 8, 7, 7);
        ofSetColor(sensors.hokuyos[i]->sensorColor);
        sensorFont.drawString(sensorString, x + 15, y + i * 20);
    }
}

void Viewer::drawSensor(Hokuyo* hokuyo) {
    ofPoint point = ofPoint(hokuyo->position.x, hokuyo->position.y);
    
    point *= scale;
    point += space.origin;
    
    float size = hokuyo->mouseBoxSize;
    float halfSize = hokuyo->mouseBoxHalfSize;
    float noseRadius = hokuyo->mouseNoseBoxRadius;
    float noseSize = hokuyo->mouseNoseBoxSize;

    ofSetColor(hokuyo->sensorColor);
    if (hokuyo->isMouseOver) {
        ofFill();
    } else {
        ofNoFill();
    }
    
    ofPushMatrix();
    ofTranslate(point.x, point.y);
    ofRotateRad(hokuyo->sensorRotationRad);
    ofDrawRectangle(-halfSize, -halfSize, size, size);
    
    if (hokuyo->isConnected) {
        float time = fmod(ofGetElapsedTimef(), 1.0);
        for (int i = 0; i < 3; i++) {
            float t = 1.0 / 3.0 * i;
            float s = ofMap(fmod((time + t), 1.0), 0.0, 1.0, 0, size);
            ofRectangle r;
            r.setFromCenter(0, 0, s, s);
            ofDrawRectangle(r);
        }
    }
    
    if (hokuyo->isMouseOverNose) {
        ofFill();
    } else {
        ofNoFill();
    }
        
    ofRectangle nose;
    nose.setFromCenter(0, noseRadius, noseSize / 2.0, noseSize / 2.0);
    ofDrawRectangle(nose);
    ofDrawLine(0, halfSize, 0, size + halfSize);

    ofPopMatrix();
}

void Viewer::drawDraggablePoints(Bounds& bounds) {
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
