//
//  Bounds.cpp
//

#include "Bounds.hpp"

Bounds::Bounds() {
    x1 = 0.0;
    x2 = 0.0;
    y1 = 0.0;
    y2 = 0.0;
    
    selectedDraggablePointIndex = -1;
    highlightedDraggablePointIndex = -1;
    mouseBoxSize = 12;
    mouseBoxHalfSize = mouseBoxSize * 0.5;
    draggablePoints.resize(4);
    
    ofAddListener(ofEvents().mouseMoved, this, &Bounds::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &Bounds::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &Bounds::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &Bounds::onMouseReleased);
}

void Bounds::setBounds(float _x1, float _x2, float _y1, float _y2) {
    x1 = _x1;
    x2 = _x2;
    y1 = _y1;
    y2 = _y2;
    
    updateDraggablePoints();
}

void Bounds::setSpace(Space & _space) {
    space = _space;
    scale = space.width / (space.areaSize * 1000.0);
}

void Bounds::updateDraggablePoints() {
    float bx = x1 * 1000.0 * scale + space.origin.x;
    float by = y1 * 1000.0 * scale + space.origin.y;
    float bw = fabs(x2 - x1) * 1000.0 * scale;
    float bh = fabs(y2 - y1) * 1000.0 * scale;
    
    draggablePoints[0] = ofPoint(bx, by + bh / 2.0);
    draggablePoints[1] = ofPoint(bx + bw / 2.0, by);
    draggablePoints[2] = ofPoint(bx + bw, by + bh / 2.0);
    draggablePoints[3] = ofPoint(bx + bw / 2.0, by + bh);
}

void Bounds::updateBounds() {
    x1 = (draggablePoints[0].x - space.origin.x) / scale * 0.001;
    y1 = (draggablePoints[1].y - space.origin.y) / scale * 0.001;
    x2 = (draggablePoints[2].x - space.origin.x) / scale * 0.001;
    y2 = (draggablePoints[3].y - space.origin.y) / scale * 0.001;
}

void Bounds::onMouseMoved(ofMouseEventArgs& mouseArgs) {
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

void Bounds::onMousePressed(ofMouseEventArgs& mouseArgs) {
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

void Bounds::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    if (selectedDraggablePointIndex >= 0) {
        draggablePoints[selectedDraggablePointIndex].set(mousePoint);
        updateBounds();
        updateDraggablePoints();
    }
}

void Bounds::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    if (selectedDraggablePointIndex >= 0) {
        draggablePoints[selectedDraggablePointIndex].set(mousePoint);
        updateBounds();
        updateDraggablePoints();
    }
}
