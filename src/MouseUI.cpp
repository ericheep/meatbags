//
//  MouseUI.cpp
//

#include "MouseUI.hpp"

MouseUI::MouseUI() {
    ofAddListener(ofEvents().mouseMoved, this, &MouseUI::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &MouseUI::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &MouseUI::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &MouseUI::onMouseReleased);

    draggableBoundsPositions.resize(4);
}

void MouseUI::update() {
    
}

void MouseUI::setBounds(Bounds & _bounds) {
    bounds = _bounds;
}

void MouseUI::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < 4; i++) {
        ofPoint & draggablePoint = draggableBoundsPositions[i];
        if(mousePoint.distance(draggablePoint) <= bounds.mouseBoxHalfSize) {
            draggablePoint.set(mousePoint);
            bounds.selectedDraggablePointIndex = i;
            break;
        }
    }

}

void MouseUI::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < 4; i++) {
        ofPoint & draggablePoint = draggableBoundsPositions[i];
        if(mousePoint.distance(draggablePoint) <= bounds.mouseBoxHalfSize) {
            draggablePoint.set(mousePoint);
            bounds.selectedDraggablePointIndex = i;
            return;
        }
    }
    
    selectedDraggablePointIndex = -1;
}

void MouseUI::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    if (selectedDraggablePointIndex >= 0) {
        draggableBoundsPositions[selectedDraggablePointIndex].set(mousePoint);
        //updateBounds();
        //updateDraggablePoints();
    }
}

void MouseUI::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    if (selectedDraggablePointIndex >= 0) {
        draggableBoundsPositions[selectedDraggablePointIndex].set(mousePoint);
        //updateBounds();
        //updateDraggablePoints();
    }
}
