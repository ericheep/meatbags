//
//  Bounds.cpp
//

#include "Bounds.hpp"

Bounds::Bounds() {
    
}

Bounds::~Bounds() {
    ofRemoveListener(ofEvents().mouseMoved, this, &Bounds::onMouseMoved);
    ofRemoveListener(ofEvents().mousePressed, this, &Bounds::onMousePressed);
    ofRemoveListener(ofEvents().mouseDragged, this, &Bounds::onMouseDragged);
    ofRemoveListener(ofEvents().mouseReleased, this, &Bounds::onMouseReleased);
}

Bounds::Bounds(int numberPoints) {
    points.resize(numberPoints);
    positions.resize(numberPoints);
    
    for (int i = 0; i < numberPoints; i++) {
        positions[i].size = 15;
        positions[i].halfSize = positions[i].size * 0.5;
        positions[i].isMouseOver = false;
        positions[i].isMouseClicked = false;
    }
    
    ofAddListener(ofEvents().mouseMoved, this, &Bounds::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &Bounds::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &Bounds::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &Bounds::onMouseReleased);
}

void Bounds::update() {
    polyline.clear();
    for (auto point : points) {
        polyline.addVertex(ofPoint(point) * 1000);
    }
    for(int i = 0; i < positions.size(); i++) {
        //positions[i].x = points[i]->x;
        //positions[i].y = points[i]->y;
    }
    polyline.close();
}

void Bounds::setPoint(int index, ofVec2f point) {
    // points[index] = point;

    polyline.clear();
    for (auto point : points) {
        polyline.addVertex(ofPoint(point));
    }
    polyline.close();
}

void Bounds::setSpace(Space & _space) {
    space = _space;
    scale = space.width / (space.areaSize * 1000.0);
}

ofPoint Bounds::convertCoordinateToScreenPoint(ofPoint coordinate) {
    return coordinate * scale + space.origin;
}

ofPoint Bounds::convertScreenPointToCoordinate(ofPoint screenPoint) {
    return (screenPoint - space.origin) / scale * 0.001;
}

void Bounds::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < positions.size(); i++) {
        ofPoint position = ofPoint(positions[i].x, positions[i].y);
        ofPoint screenPoint = convertCoordinateToScreenPoint(position);

        if(mousePoint.distance(screenPoint) <= positions[i].halfSize) {
            positions[i].isMouseOver = true;
        } else {
            positions[i].isMouseOver = false;
        }
    }
}

void Bounds::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    /*for(int i = 0; i < positions.size(); i++) {
        ofPoint screenPoint = convertCoordinateToScreenPoint(positions[i]);

        if(mousePoint.distance(screenPoint) <= positions[i].halfSize) {
            positions[i].isMouseClicked = true;
        } else {
            positions[i].isMouseClicked = false;
        }
    }*/
}

void Bounds::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < positions.size(); i++) {
        // cout << draggablePoints[i].isMouseClicked << endl;
        if(positions[i].isMouseClicked) {
            cout << positions.size() << endl;

           // ofPoint coordinate = convertCoordinateToScreenPoint(mousePoint);
            //points[i] = coordinate;
        }
    }
}

void Bounds::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < positions.size(); i++) {
        //if (positions[i].isMouseClicked) {
           // positions[i].isMouseClicked = false;
        //}
    }
}
