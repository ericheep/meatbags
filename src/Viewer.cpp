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
    filterFont.load(ofToDataPath("Hack-Bold.ttf"), 12);
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
        
        for (int j = -3; j < (int) space.areaSize + 6; j++) {
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

bool Viewer::checkWithinBounds(float x, float y, Filters & filters) {
    bool isWithinFilter = false;
    for (auto filter : filters.filters) {
        if (!filter->mask) {
            if (filter->polyline.inside(x * 0.001, y * 0.001)) {
                isWithinFilter = true;
            }
        }
    }
    
    for (auto filter : filters.filters) {
        if (filter->mask) {
            if (filter->polyline.inside(x * 0.001, y * 0.001)) {
                isWithinFilter = false;
            }
        }
    }
    
    return isWithinFilter;
}


void Viewer::drawCoordinates(vector<ofPoint>& coordinates, ofColor color, Filters & filters) {
    for (auto& coordinate : coordinates) {
        ofColor pointColor;

        float x = coordinate.x;
        float y = coordinate.y;
        
        if (x == 0 && y == 0) break;

        if (checkWithinBounds(x, y, filters)) {
            pointColor.set(color.r, color.g, color.b, 255);
        } else {
            pointColor.set(color.r, color.g, color.b, 90);
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

void Viewer::drawSensors(Sensors& sensors, Filters & filters) {
    for (auto& sensor : sensors.hokuyos) {
        drawCoordinates(sensor->coordinates, sensor->sensorColor, filters);
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
    
    float size = hokuyo->position.size;
    float halfSize = hokuyo->position.halfSize;
    float noseRadius = hokuyo->noseRadius;
    float noseSize = hokuyo->nosePosition.size;

    ofSetColor(hokuyo->sensorColor);
    if (hokuyo->position.isMouseOver) {
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
    
    if (hokuyo->nosePosition.isMouseOver) {
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

void Viewer::drawDraggablePoints(Filter & bounds) {
    for (auto position : bounds.positions) {
        ofPoint point = position * 1000.0 * scale + space.origin;

        ofRectangle p;
        float r = position.size;
        p.setFromCenter(point.x, point.y, r, r);
        ofNoFill();
        
        if (position.isMouseOver) ofFill();
        
        ofSetColor(ofColor::magenta);
        ofDrawRectangle(p);
        
        if (bounds.centroid.isMouseOver) {
            ofPoint centroidPoint = bounds.centroid * 1000.0 * scale + space.origin;
            ofDrawLine(point.x, point.y, centroidPoint.x, centroidPoint.y);
        }
    }
    
    if (bounds.centroid.isMouseOver) {
        ofPoint centroidPoint = bounds.centroid * 1000.0 * scale + space.origin;
        
        ofRectangle p;
        float r = bounds.centroid.size;
        p.setFromCenter(centroidPoint.x, centroidPoint.y, r, r);
        
        ofFill();
        
        ofSetColor(ofColor::magenta);
        ofDrawRectangle(p);
    }
}

void Viewer::drawFilters(Filters & filters) {
    for (auto & filter : filters.filters) {
        drawFilter(filter);
    }
}

void Viewer::drawFilter(Filter * filter) {
    ofNoFill();
    
    ofColor filterColor = ofColor::magenta;
    if (filter->mask) filterColor = ofColor::lightPink;
    ofSetColor(filterColor);
    
    ofPolyline p;
    for (auto vertex : filter->polyline.getVertices()) {
        p.addVertex(vertex * scale * 1000.0 + space.origin);
    }
    p.close();
    p.draw();
    
    ofColor shapeColor = ofColor(filterColor.r, filterColor.g, filterColor.b, 40);
    ofSetColor(shapeColor);
    
    ofFill();
    if (filter->mask) {
        ofBeginShape();
        for(auto & position : filter->positions) {
            ofPoint p = position * scale * 1000.0 + space.origin;
            ofVertex(p);
        }
        ofEndShape();
    }
    
    drawDraggablePoints(filter);
}

void Viewer::drawDraggablePoints(Filter * filter) {
    ofColor filterColor = ofColor::magenta;
    if (filter->mask) filterColor = ofColor::lightPink;
    
    for (auto position : filter->positions) {
        ofPoint point = position * 1000.0 * scale + space.origin;
        
        ofRectangle p;
        float r = position.size;
        p.setFromCenter(point.x, point.y, r, r);
        ofNoFill();
        
        if (position.isMouseOver) ofFill();
        
        ofSetColor(filterColor);
        ofDrawRectangle(p);
        
        if (filter->centroid.isMouseOver) {
            ofPoint centroidPoint = filter->centroid * 1000.0 * scale + space.origin;
            ofDrawLine(point.x, point.y, centroidPoint.x, centroidPoint.y);
        }
    }
    
    ofSetColor(filterColor);
    ofPoint indexPoint = filter->centroid * 1000.0 * scale + space.origin;
    string indexString = to_string(filter->index);
    float xOffset = filterFont.stringWidth(indexString) * 0.5;
    float yOffset = filterFont.stringHeight(indexString) * 0.5;
    filterFont.drawString(to_string(filter->index), indexPoint.x - xOffset, indexPoint.y + yOffset);

    if (filter->centroid.isMouseOver) {
        ofPoint centroidPoint = filter->centroid * 1000.0 * scale + space.origin;
        
        ofRectangle p;
        float r = filter->centroid.size;
        p.setFromCenter(centroidPoint.x, centroidPoint.y, r, r);
        
        ofFill();
        
        ofSetColor(filterColor);
        ofDrawRectangle(p);
    }
}
