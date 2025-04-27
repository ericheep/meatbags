//
//  Bounds.cpp
//

#include "Bounds.hpp"

Bounds::Bounds() {
    ofAddListener(ofEvents().mouseMoved, this, &Bounds::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &Bounds::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &Bounds::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &Bounds::onMouseReleased);
}

Bounds::~Bounds() {
    ofRemoveListener(ofEvents().mouseMoved, this, &Bounds::onMouseMoved);
    ofRemoveListener(ofEvents().mousePressed, this, &Bounds::onMousePressed);
    ofRemoveListener(ofEvents().mouseDragged, this, &Bounds::onMouseDragged);
    ofRemoveListener(ofEvents().mouseReleased, this, &Bounds::onMouseReleased);
}

void Bounds::setNumberPoints(int numberPoints) {
    points.resize(numberPoints);
    positions.resize(numberPoints);
    
    for (int i = 0; i < numberPoints; i++) {
        positions[i].size = 15;
        positions[i].halfSize = positions[i].size * 0.5;
        positions[i].isMouseOver = false;
        positions[i].isMouseClicked = false;
    }
}

void Bounds::update() {
    polyline.clear();
    for (auto point : points) {
        polyline.addVertex(ofPoint(point) * 1000);
    }
    for(int i = 0; i < positions.size(); i++) {
        positions[i].x = points[i]->x;
        positions[i].y = points[i]->y;
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
        ofPoint screenPoint = convertCoordinateToScreenPoint(positions[i]);
        if (i == 0) cout << screenPoint << " " << mousePoint << endl;
        if(mousePoint.distance(screenPoint) <= positions[i].halfSize) {
            positions[i].isMouseOver = true;
        } else {
            positions[i].isMouseOver = false;
        }
    }
}

void Bounds::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < positions.size(); i++) {
        ofPoint screenPoint = convertCoordinateToScreenPoint(positions[i]);

        if(mousePoint.distance(screenPoint) <= positions[i].halfSize) {
            positions[i].isMouseClicked = true;
        } else {
            positions[i].isMouseClicked = false;
        }
    }
}

void Bounds::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < positions.size(); i++) {
        if(positions[i].isMouseClicked) {
            ofPoint coordinate = convertCoordinateToScreenPoint(mousePoint);
            points[i] = coordinate;
        }
    }
}

void Bounds::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < positions.size(); i++) {
        if (positions[i].isMouseClicked) {
           positions[i].isMouseClicked = false;
        }
    }
}
