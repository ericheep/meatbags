//
//  Viewer.cpp
//

#include "Viewer.hpp"

Viewer::Viewer() {
    scale = 0.0;
    translation = ofPoint::zero();
    
    mesh.getVertices().reserve(50000);
    mesh.getColors().reserve(50000);
    mesh.getIndices().reserve(50000);

    circleResolution = 5;
    initializeCircleMeshes();
    
    blobFont.setBold();
    blobFont.setSize(14);
    sensorFont.setBold();
    sensorFont.setSize(13);
    filterFont.setBold();
    filterFont.setSize(12);
    cursorFont.setBold();
    cursorFont.setSize(12);
    titleFont.setBold();
    titleFont.setSize(14);
    helpFont.setMedium();
    helpFont.setSize(14);
    saveFont.setBold();
    saveFont.setSize(18);
}

Viewer::~Viewer() {
}

void Viewer::setSpace(const Space& _space) {
    space = _space;
    scale = space.width / (space.areaSize * 1000);
}

void Viewer::setTranslation(ofPoint _translation) {
    translation = _translation;
}

void Viewer::draw(const vector<Blob>& blobs, const vector<Filter*>& filters, const vector<Sensor*>& sensors) {
    ofPushMatrix();
    ofTranslate(translation);
    drawGrid();
    drawBlobs(blobs);
    drawFilters(filters);
    drawSensors(sensors);
    ofPopMatrix();
    
    for (const auto& sensor : sensors) {
        if (sensor->showSensorInformation) sensor->draw();
    }
    
    drawConnections(sensors);
    drawCursorCoordinate();
}

void Viewer::drawCoordinates(vector<LidarPoint>& lidarPoints, int numberLidarPoints) {
    initializeTrianglesMesh(lidarPoints, numberLidarPoints);
    ofPushMatrix();
    ofTranslate(translation);
    mesh.draw();
    ofPopMatrix();
}

void Viewer::drawGrid() {
    
    /*float crossHalfLength = scale * 50;
     for (int i = -25; i < 25; i++) {
     for (int j = -10; j < 50; j++) {
     float x = i * 1000.0 * scale + space.origin.x;
     float y = j * 1000.0 * scale + space.origin.y;
     
     for (int k = 0; k < 3; k++) {
     float ex = ofMap(k, 0, 3 - 1, x - scale * 333, x + scale * 333);
     float ey = ofMap(k, 0, 3 - 1, y - scale * 333, y + scale * 333);
     
     ofDrawLine(ex - crossHalfLength, y, ex + crossHalfLength, y);
     ofDrawLine(x, ey - crossHalfLength, x, ey + crossHalfLength);}}}*/
    
    float xScalar = 1000.0 * scale;
    float height = 3000;
    for (int i = -30; i < 30; i++) {
        float x = i * xScalar + space.origin.x;
        if (i % 5 == 0) {
            ofSetColor(ofColor(75, 75, 75));
        } else {
            ofSetColor(ofColor(45, 45, 45));
        }
        ofDrawLine(x, -height, x, height);
    }
    
    float yScalar = 1000.0 * scale;
    float width = 3000;
    for (int i = -30; i < 30; i++) {
        float y = i * yScalar + space.origin.y;
        if (i % 5 == 0) {
            ofSetColor(ofColor(75, 75, 75));
        } else {
            ofSetColor(ofColor(45, 45, 45));
        }
        ofDrawLine(-width, y, width, y);
    }
    
    ofSetColor(ofColor::thistle);
    
    ofFill();
    ofSetCircleResolution(23);
    ofDrawCircle(space.origin, 4);
}

void Viewer::drawCursorCoordinate() {
    float x = ofGetWidth() - 90;
    float y = ofGetHeight() - 20;
    float length = cursorFont.getStringWidth(cursorString);
    float height = 20;
    
    ofColor cursorBackgroundColor = ofColor::black;
    cursorBackgroundColor.a = 160;
    ofFill();
    ofSetColor(cursorBackgroundColor);
    
    ofDrawRectangle(x, y - 15, length, height);
    
    ofSetColor(ofColor::thistle);
    cursorFont.draw(cursorString, x, y);
}

void Viewer::drawBlobs(const vector<Blob>& blobs) {
    ofNoFill();
    ofSetColor(0, 0, 255);
    
    for (const auto& blob : blobs) {
        float scaledX = blob.centroid.x * scale + space.origin.x;
        float scaledY = blob.centroid.y * scale + space.origin.y;
        
        float boxX = blob.bounds.getX() * scale + space.origin.x;
        float boxY = blob.bounds.getY() * scale + space.origin.y;
        float boxW = blob.bounds.getWidth() * scale;
        float boxH = blob.bounds.getHeight() * scale;
        
        ofDrawRectangle(boxX, boxY, boxW, boxH);
        
        ofFill();
        ofSetColor(255, 0, 0);
        ofDrawEllipse(scaledX, scaledY, 9, 9);
        
        char buffer[32];
        sprintf(buffer, "%d", blob.index);
        blobFont.draw(buffer, scaledX + 15, scaledY - 21);
        
        sprintf(buffer, "x: %.2f", blob.centroid.x / 1000.0);
        blobFont.draw(buffer, scaledX + 15, scaledY - 7);
        
        sprintf(buffer, "y: %.2f", blob.centroid.y / 1000.0);
        blobFont.draw(buffer, scaledX + 15, scaledY + 7);
        
        sprintf(buffer, "points: %d", blob.numberPoints);
        blobFont.draw(buffer, scaledX + 15, scaledY + 21);
        
        ofNoFill();
        ofSetColor(0, 0, 255);
    }
}

void Viewer::initializeTrianglesMesh(const vector<LidarPoint>& lidarPoints, int numberLidarPoints) {
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    
    float radius = 0.0;
    for (int i = 0; i < numberLidarPoints; i++) {
        ofColor pointColor = lidarPoints[i].color;
        
        if (lidarPoints[i].isInFilter) {
            radius = 2.5;
            pointColor.a = 255;
        } else {
            radius = 1.5;
            pointColor.a = 90;
        }
        
        ofPoint coordinate = lidarPoints[i].coordinate;
        coordinate = coordinate * scale + space.origin;
        
        mesh.addVertex(coordinate);
        mesh.addColor(pointColor);
        for (int j = 0; j < circleResolution; j++) {
            mesh.addVertex(coordinate + circleMesh.getVertex(j) * radius);
            mesh.addColor(pointColor);
        }
    }
    
    for (int i = 0; i < numberLidarPoints; i++) {
        int coordinateIndex = (circleResolution + 1) * i;
        
        for (int j = 0; j < circleResolution; j++) {
            mesh.addIndex(coordinateIndex);
            mesh.addIndex(coordinateIndex + j + 1);
            
            if (j < circleResolution - 1) {
                mesh.addIndex(coordinateIndex + j + 2);
            } else {
                mesh.addIndex(coordinateIndex + 1);
            }
        }
    }
}

void Viewer::initializeCircleMeshes() {
    circleMesh.clear();
    float deltaTheta = TWO_PI / float(circleResolution);
    
    for (float i = 0; i < TWO_PI; i = i + deltaTheta) {
        float x = cos(i);
        float y = sin(i);
        circleMesh.addVertex(ofVec3f(x, y, 0));
    }
}

void Viewer::drawSensors(const vector<Sensor*>& sensors) {
    for (const auto& sensor : sensors) {
        drawSensor(sensor);
    }
}

void Viewer::drawConnections(const vector<Sensor*>& sensors) {
    int numberSensors = sensors.size();
    
    float connectionsBoxHeight = numberSensors * 20;
    float y = space.height - connectionsBoxHeight + 8;
    float x = 10;
    
    for (int i = 0; i < numberSensors; i++) {
        bool connected = sensors[i]->isConnected;
        string model = sensors[i]->model;
        
        ofFill();
        if (connected) {
            ofSetColor(0, 255, 0, 130);
        } else {
            ofSetColor(255, 0, 0, 130);
        }
        
        string sensorString = "Sensor " + to_string(i + 1) + ": " + model;
        ofDrawRectangle(x, y + i * 20 - 8, 7, 7);
        ofSetColor(sensors[i]->sensorColor);
        sensorFont.draw(sensorString, x + 15, y + i * 20 - 1);
    }
}

void Viewer::drawSensor(const Sensor* sensor) {
    ofPoint point = ofPoint(sensor->position.x, sensor->position.y);
    
    point *= scale;
    point += space.origin;
    
    float size = sensor->position.size;
    float halfSize = sensor->position.halfSize;
    float noseRadius = sensor->noseRadius;
    float noseSize = sensor->nosePosition.size;
    
    ofSetColor(sensor->sensorColor);
    if (sensor->position.isMouseOver) {
        ofFill();
    } else {
        ofNoFill();
    }
    
    ofPushMatrix();
    ofTranslate(point.x, point.y);
    ofRotateRad(sensor->sensorRotationRad);
    ofDrawRectangle(-halfSize, -halfSize, size, size);
    
    if (sensor->isConnected) {
        float time = fmod(ofGetElapsedTimef(), 1.0);
        for (int i = 0; i < 3; i++) {
            float t = 1.0 / 3.0 * i;
            float s = ofMap(fmod((time + t), 1.0), 0.0, 1.0, 0, size);
            ofRectangle r;
            r.setFromCenter(0, 0, s, s);
            ofDrawRectangle(r);
        }
    }
    
    if (sensor->nosePosition.isMouseOver) {
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

void Viewer::drawDraggablePoints(const Filter& bounds) {
    for (const auto& draggablePoint : bounds.draggablePoints) {
        ofPoint point = draggablePoint * 1000.0 * scale + space.origin;
        
        ofRectangle p;
        float r = draggablePoint.size;
        p.setFromCenter(point.x, point.y, r, r);
        ofNoFill();
        
        if (draggablePoint.isMouseOver) ofFill();
        
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

void Viewer::drawFilters(const vector<Filter*>& filters) {
    for (auto & filter : filters) {
        drawFilter(filter);
    }
}

void Viewer::drawFilter(Filter * filter) {
    ofColor filterColor = ofColor::magenta;
    
    if (!filter->isBlobInside) filterColor.a = 150;
    if (filter->isMask) filterColor = ofColor::lightPink;
    if (!filter->isActive) filterColor.lerp(ofColor::grey, 0.95);
    
    ofNoFill();
    ofSetColor(filterColor);
    filter->drawOutline();
    
    ofColor shapeColor = ofColor(filterColor.r, filterColor.g, filterColor.b, 40);
    ofSetColor(shapeColor);
    ofFill();
    if (filter->isMask) filter->drawShape();
    
    drawDraggablePoints(filter);
}

void Viewer::drawDraggablePoints(const Filter* filter) {
    ofColor filterColor = ofColor::magenta;
    if (!filter->isBlobInside) filterColor.a = 150;
    if (filter->isMask) filterColor = ofColor::lightPink;
    if (!filter->isActive) filterColor.lerp(ofColor::grey, 0.95);
    
    for (auto& draggablePoint : filter->draggablePoints) {
        ofPoint point = draggablePoint * 1000.0 * scale + space.origin;
        
        ofRectangle p;
        float r = draggablePoint.size;
        p.setFromCenter(point.x, point.y, r, r);
        ofNoFill();
        
        if (draggablePoint.isMouseOver) ofFill();
        
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
    
    filterFont.draw(to_string(filter->index), indexPoint.x - 3, indexPoint.y + 3);
    
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

void Viewer::setCursorString(const ofPoint& mousePoint) {
    ofPoint point = (mousePoint - space.origin - translation) / scale * 0.001;
     
    char buffer[32];
    sprintf(buffer, "%.3f %.3f", point.x, point.y);
    cursorString = buffer;
}

void Viewer::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint = ofPoint(mouseArgs.x, mouseArgs.y);
    setCursorString(mousePoint);
}

void Viewer::onMouseDragged(ofMouseEventArgs &mouseArgs) {
    ofPoint mousePoint = ofPoint(mouseArgs.x, mouseArgs.y);
    setCursorString(mousePoint);
}

void Viewer::drawHelpText() {
    ofSetColor(ofColor::thistle);
    
    titleFont.draw("meatbags v" + version, 15, 20);
    helpFont.draw("headless mode", 15, 40);
    
    helpFont.draw("(h) toggle help file", 15, 80);
    helpFont.draw("(m) hold and move mouse to translate grid", 15, 100);
    helpFont.draw("(f) press while over the center of a filter to toggle mask/filter", 15, 120);
    helpFont.draw("(t) press while over the center of a filter to toggle active/inactive", 15, 140);
    helpFont.draw("(ctrl/cmd + s) press to save", 15, 160);
    
    titleFont.draw("blob OSC format", 15, 200);
    helpFont.draw("/blob index x y width length laserIntensity filterIndex1 filterIndex2 ...", 15, 220);
    helpFont.draw("/blobsActive index1 index2 ...", 15, 240);
    
    titleFont.draw("filter OSC format", 15, 280);
    helpFont.draw("/filter index isAnyBlobInside blobDistanceToCentroid", 15, 300);
    helpFont.draw("/filterBlobs filterIndex blobIndex1 x1 y1 blobIndex2 x2 y2 ...", 15, 320);
    
    titleFont.draw("logging OSC format", 15, 360);
    helpFont.draw("/generalStatus sensorIndex status", 15, 380);
    helpFont.draw("/connectionStatus sensorIndex status", 15, 400);
    helpFont.draw("/laserStatus sensorIndex status", 15, 420);
}

void Viewer::drawSaveNotification() {
    string saveText = "configuration saved";
    float stringWidth = saveFont.getStringWidth(saveText);
    ofRectangle saveRectangle;
    saveRectangle.setFromCenter(ofGetWidth() * 0.5, ofGetHeight() * 0.5 - 5, stringWidth + 50, 35);
    
    ofSetColor(ofColor::black);
    ofDrawRectangle(saveRectangle);
    ofSetColor(ofColor::thistle);
    ofNoFill();
    ofDrawRectangle(saveRectangle);
    saveFont.draw(saveText, ofGetWidth() * 0.5 - stringWidth * 0.5, ofGetHeight() * 0.5);
}
