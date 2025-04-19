//
//  Meatbags.cpp
//

#include "Meatbags.hpp"

Meatbags::Meatbags() {
    polarCoordinates.resize(1024);
    cartesianCoordinates.resize(1024);
    intensities.resize(1024);
    filteredCoordinates.resize(1024);
    filteredIntensities.resize(1024);
    draggablePoints.resize(4);
    
    epsilon = 100.0;
    minPoints = 5;
    mouseBoxSize = 8;
    mouseBoxHalfSize = mouseBoxSize * 0.5;
    pixelsPerUnit = 0;
    selectedDraggablePointIndex = 0;
    numberFilteredCoordinates = 0;
    
    boundsX1 = -2.5;
    boundsX2 = 2.5;
    boundsY1 = 0;
    boundsY2 = 5;
    
    ofAddListener(ofEvents().mouseMoved, this, &Meatbags::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &Meatbags::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &Meatbags::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &Meatbags::onMouseReleased);
    
    loadFile("meatbags.xml");
}

void Meatbags::update() {
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
    int filteredIndex = 0;
    for (int i = 0; i < cartesianCoordinates.size(); i++) {
        float x = cartesianCoordinates[i].x;
        float y = cartesianCoordinates[i].y;
        
        if (x != 0 && y != 0) {
            if (x < boundsX2 * 1000
                && x > boundsX1 * 1000
                && y > boundsY1 * 1000
                && y < boundsY2 * 1000)
            {
                filteredCoordinates[filteredIndex].set(x, y);
                filteredIntensities[filteredIndex] = intensities[i];
                filteredIndex++;
            }
        }
    }
    
    numberFilteredCoordinates = filteredIndex;
}

float Meatbags::pointDistance(ofPoint a, ofPoint b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

void Meatbags::clusterBlobs() {
    vector<struct point2> points;
    for (int i = 0; i < numberFilteredCoordinates; i++) {
        struct point2 point;
        point.x = filteredCoordinates[i].x;
        point.y = filteredCoordinates[i].y;
        points.push_back(point);
    }
    
    auto clusters = dbscan(points, epsilon, minPoints);
    
    // get new blobs
    newBlobs.clear();
    int index = 0;
    for(auto& cluster: clusters) {
        vector<ofPoint> coordinates;
        vector<int> intensities;
        for (int i = 0; i < cluster.size(); i++) {
            int coordinateIndex = cluster[i];
            coordinates.push_back(filteredCoordinates[coordinateIndex]);;
            intensities.push_back(filteredIntensities[coordinateIndex]);
        }
        
        Blob newBlob = Blob(coordinates, intensities);
        newBlob.index = index;
        newBlobs.push_back(newBlob);
        
        index++;
    }
    
    // if no olds blobs present then we use all the new blobs
    if (oldBlobs.size() == 0) {
        int index = 0;
        for (auto& newBlob : newBlobs) {
            Blob blob = Blob();
            blob.become(newBlob);
            blob.index = index;
            oldBlobs.push_back(blob);
            
            index++;
        }
    }
    
    // initialization
    for (auto& oldBlob : oldBlobs) {
        oldBlob.setMatched(false);
    }
}

float Meatbags::compareBlobs(Blob newBlob, Blob oldBlob) {
    float epsilon = 1;
    float distance = pointDistance(newBlob.centroid, oldBlob.centroid);
    return 1.0 / (distance + epsilon);
}

void Meatbags::matchBlobs() {
    for (auto& newBlob : newBlobs) {
        int potentialMatchIndex = 0;
        float potentialHighestScore = 0.0;
        
        for (auto& oldBlob : oldBlobs) {
            float score = compareBlobs(newBlob, oldBlob);
            
            if (score > potentialHighestScore) {
                potentialMatchIndex = oldBlob.index;
                potentialHighestScore = score;
            }
        }
        
        newBlob.setPotentialMatch(potentialMatchIndex, potentialHighestScore);
    }
    
    sort(newBlobs.begin(), newBlobs.end(), [](const Blob& a, const Blob& b){
        return a.potentialMatchScore > b.potentialMatchScore;
    });
    
    for (auto& newBlob : newBlobs) {
        for (auto& oldBlob : oldBlobs) {
            if (oldBlob.index == newBlob.potentialMatchIndex && !newBlob.isMatched() && !oldBlob.isMatched())             {
                oldBlob.become(newBlob);
                
                newBlob.setMatched(true);
                oldBlob.setMatched(true);
            }
        }
    }
}

void Meatbags::removeBlobs() {
    oldBlobs.erase(std::remove_if(oldBlobs.begin(), oldBlobs.end(), [](Blob blob) {
        return !blob.isMatched();
    }), oldBlobs.end());
}

int Meatbags::findFreeBlobIndex() {
    int freeIndex = 0;
    Boolean lookingForFreeIndex = true;
    
    while (lookingForFreeIndex) {
        lookingForFreeIndex = false;
        
        for (Blob oldBlob : oldBlobs) {
            if (freeIndex == oldBlob.index) lookingForFreeIndex = true;
        }
        
        if (lookingForFreeIndex) freeIndex++;
    }
    
    return freeIndex;
}

void Meatbags::addBlobs() {
    for (auto& newBlob : newBlobs) {
        if (!newBlob.isMatched()) {
            Blob blob;
            
            int freeIndex = findFreeBlobIndex();
            
            blob.index = freeIndex;
            blob.setMatched(true);
            blob.become(newBlob);
            
            oldBlobs.push_back(blob);
        }
    }
}

void Meatbags::calculateBlobs() {
    if (filteredCoordinates.size() == 0) return;
    
    clusterBlobs();
    matchBlobs();
    removeBlobs();
    addBlobs();
}

void::Meatbags::getBlobs(vector<Blob> &blobs) {
    blobs.clear();
    for (auto& oldBlob : oldBlobs) {
        blobs.push_back(oldBlob);
    }
}

void Meatbags::drawGrid() {
    ofSetColor(ofColor::grey);
    float crossHalfLength = scale * 25;

    for (int i = 0; i < (int) areaSize + 2; i++) {
        int gridLineIndex = i + -( (int) areaSize + 2) / 2;
        
        for (int j = 0; j < (int) areaSize + 2; j++) {
            float x = gridLineIndex * 1000.0 * scale + puckPosition.x;
            float y = j * 1000.0 * scale + puckPosition.y;
        
            for (int k = 0; k < 3; k++) {
                float ex = ofMap(k, 0, 3 - 1, x - scale * 333, x + scale * 333);
                float ey = ofMap(k, 0, 3 - 1, y - scale * 333, y + scale * 333);
                
                ofDrawLine(ex - crossHalfLength, y, ex + crossHalfLength, y);
                ofDrawLine(x, ey - crossHalfLength, x, ey + crossHalfLength);
            }
        }
    }
}

void Meatbags::drawScanningPoints() {
    ofNoFill();
    ofSetColor(255);
    ofDrawRectangle(0, 0, width, height);
    
    for (int i = 0; i < cartesianCoordinates.size(); i++) {
        float x = cartesianCoordinates[i].x;
        float y = cartesianCoordinates[i].y;
        
        if (x < boundsX2 * 1000
            && x > boundsX1 * 1000
            && y > boundsY1 * 1000
            && y < boundsY2 * 1000)
        {
            ofSetColor(255);
        } else {
            ofSetColor(100);
        }
        
        x *= scale;
        y *= scale;
        
        float x2 = x + puckPosition.x;
        float y2 = y + puckPosition.y;
        
        ofFill();
        ofDrawEllipse(x2, y2, 3, 3);
    }
}

void Meatbags::drawBlobBounds() {
    ofNoFill();
    ofSetColor(ofColor::magenta);
    float bx = boundsX1 * 1000 * scale + puckPosition.x;
    float by = boundsY1 * 1000 * scale + puckPosition.y;
    float bw = fabs(boundsX2 * 1000 - boundsX1 * 1000) * scale;
    float bh = fabs(boundsY2 * 1000 - boundsY1 * 1000) * scale;
    ofDrawRectangle(bx, by, bw, bh);
}

void Meatbags::drawBlobs() {
    for (auto blob : oldBlobs) {
        ofRectangle blobBox = blob.bounds;
        blobBox.setX(blobBox.getX() * scale + puckPosition.x);
        blobBox.setY(blobBox.getY() * scale + puckPosition.y);
        blobBox.setWidth(blobBox.getWidth() * scale);
        blobBox.setHeight(blobBox.getHeight() * scale);
        
        ofNoFill();
        ofSetColor(0, 0, 255);
        ofDrawRectangle(blobBox);
        
        float centroidX = blob.centroid.x * scale + puckPosition.x;
        float centroidY = blob.centroid.y * scale + puckPosition.y;
        
        ofFill();
        ofSetColor(255, 0, 0);
        ofDrawEllipse(centroidX, centroidY, 9, 9);
        
        std::stringstream index;
        index << "index: " << blob.index;
        std::stringstream x;
        x << setprecision(3) << "x: " << blob.centroid.x / 1000.0 << endl;
        std::stringstream y;
        y << setprecision(3) << "y: " << blob.centroid.y / 1000.0 << endl;
       
        ofDrawBitmapString(index.str(), centroidX + 15, centroidY - 20);
        ofDrawBitmapString(x.str(), centroidX + 15, centroidY - 4);
        ofDrawBitmapString(y.str(), centroidX + 15, centroidY + 12);
        
    }
}

void Meatbags::drawDraggablePoints() {
    for (int i = 0; i < 4; i++) {
        float x1 = draggablePoints[i].x - mouseBoxHalfSize;
        float y1 = draggablePoints[i].y - mouseBoxHalfSize;
        float x2 = mouseBoxSize;
        float y2 = mouseBoxSize;
        
        ofNoFill();
        
        if (highlightedDraggablePointIndex == i) ofFill();
        
        ofSetColor(ofColor::magenta);
        ofDrawRectangle(x1, y1, x2, y2);
    }
}

void Meatbags::draw() {
    drawGrid();
    drawScanningPoints();
    drawBlobs();
    drawBlobBounds();
    drawDraggablePoints();
    
    ofSetColor(ofColor::skyBlue);
    ofFill();
    ofDrawEllipse(puckPosition.x, puckPosition.y, 10, 10);
}

void Meatbags::setCanvasSize(float _width, float _height) {
    width = _width;
    height = _height;
    puckPosition = ofPoint(width / 2.0, 25);
}

void Meatbags::setAreaSize(float _areaSize) {
    areaSize = _areaSize;
    scale = width / (areaSize * 1000);
    
    updateDraggablePoints();
}

void Meatbags::updateDraggablePoints() {
    float bx = boundsX1 * 1000.0 * scale + puckPosition.x;
    float by = boundsY1 * 1000.0 * scale + puckPosition.y;
    float bw = fabs(boundsX2 - boundsX1) * 1000.0 * scale;
    float bh = fabs(boundsY2 - boundsY1) * 1000.0 * scale;
    
    draggablePoints[0] = ofPoint(bx, by + bh / 2.0);
    draggablePoints[1] = ofPoint(bx + bw / 2.0, by);
    draggablePoints[2] = ofPoint(bx + bw, by + bh / 2.0);
    draggablePoints[3] = ofPoint(bx + bw / 2.0, by + bh);
}

void Meatbags::setBlobBounds(float _boundsX1, float _boundsX2, float _boundsY1, float _boundsY2) {
    boundsX1 = _boundsX1;
    boundsX2 = _boundsX2;
    boundsY1 = _boundsY1;
    boundsY2 = _boundsY2;
    
    updateDraggablePoints();
}

void Meatbags::updateBounds() {
    boundsX1 = (draggablePoints[0].x - puckPosition.x) / scale * 0.001;
    boundsY1 = (draggablePoints[1].y - puckPosition.y) / scale * 0.001;
    boundsX2 = (draggablePoints[2].x - puckPosition.x) / scale * 0.001;
    boundsY2 = (draggablePoints[3].y - puckPosition.y) / scale * 0.001;
}

void Meatbags::setEpsilon(float _epsilon) {
    epsilon = _epsilon;
}

void Meatbags::setMinPoints(int _minPoints) {
    minPoints = _minPoints;
}

//----------------------------------------------------- interaction.
void Meatbags::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < 4; i++) {
        ofPoint & draggablePoint = draggablePoints[i];
        if(mousePoint.distance(draggablePoint) <= mouseBoxHalfSize) {
            highlightedDraggablePointIndex = i;
            return;
        }
    }
    
    highlightedDraggablePointIndex = -1;
}

void Meatbags::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < 4; i++) {
        ofPoint & draggablePoint = draggablePoints[i];
        if(mousePoint.distance(draggablePoint) <= mouseBoxHalfSize) {
            draggablePoint.set(mousePoint);
            selectedDraggablePointIndex = i;
            return;
        }
    }
    
    selectedDraggablePointIndex = -1;
}

void Meatbags::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    if (selectedDraggablePointIndex >= 0) {
        draggablePoints[selectedDraggablePointIndex].set(mousePoint);
        updateBounds();
        updateDraggablePoints();
    }
}

void Meatbags::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    if (selectedDraggablePointIndex >= 0) {
        draggablePoints[selectedDraggablePointIndex].set(mousePoint);
        updateBounds();
        updateDraggablePoints();
    }
}

void Meatbags::saveToFile(const string& path) {
    ofXml xml;
    xml.appendChild("meatbags");
    
    ofXml bounds = xml.getChild("meatbags").appendChild("bounds");
    bounds.setAttribute("x1", ofToString(boundsX1));
    bounds.setAttribute("x2", ofToString(boundsX2));
    bounds.setAttribute("y1", ofToString(boundsY1));
    bounds.setAttribute("y2", ofToString(boundsY2));

    xml.save(path);
}

void Meatbags::loadFile(const string& path) {
    ofXml xml;
    if (!xml.load(path)) return;

    auto bounds = xml.getChild("meatbags").getChild("bounds");
    boundsX1 = bounds.getAttribute("x1").getFloatValue();
    boundsX2 = bounds.getAttribute("x2").getFloatValue();
    boundsY1 = bounds.getAttribute("y1").getFloatValue();
    boundsY2 = bounds.getAttribute("y2").getFloatValue();
}
