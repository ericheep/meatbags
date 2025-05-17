//
//  Filter.cpp
//

#include "Filter.hpp"

Filter::Filter() {
    ofAddListener(ofEvents().mouseMoved, this, &Filter::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &Filter::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &Filter::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &Filter::onMouseReleased);
    ofAddListener(ofEvents().keyPressed, this, &Filter::onKeyPressed);
    isActive = true;
}

Filter::~Filter() {
    ofRemoveListener(ofEvents().mouseMoved, this, &Filter::onMouseMoved);
    ofRemoveListener(ofEvents().mousePressed, this, &Filter::onMousePressed);
    ofRemoveListener(ofEvents().mouseDragged, this, &Filter::onMouseDragged);
    ofRemoveListener(ofEvents().mouseReleased, this, &Filter::onMouseReleased);
    ofRemoveListener(ofEvents().keyPressed, this, &Filter::onKeyPressed);
}

void Filter::setNumberPoints(int numberPoints) {
    points.resize(numberPoints);
    positions.resize(numberPoints);
    
    for (int i = 0; i < numberPoints; i++) {
        positions[i].size = 10;
        positions[i].halfSize = positions[i].size * 0.5;
        positions[i].isMouseOver = false;
        positions[i].isMouseClicked = false;
    }
    
    centroid.size = 12;
    centroid.halfSize = centroid.size * 0.5;
    centroid.isMouseOver = false;
    centroid.isMouseClicked = false;
}

void Filter::update() {
    polyline.clear();
    for (auto point : points) {
        polyline.addVertex(ofPoint(point));
    }
    polyline.close();
    
    ofPoint c = polyline.getCentroid2D();
    centroid.x = c.x;
    centroid.y = c.y;
    
    for(int i = 0; i < positions.size(); i++) {
        positions[i].x = points[i]->x;
        positions[i].y = points[i]->y;
    }
}

void Filter::checkBlobs(vector<Blob> & blobs) {
    isBlobInside = false;
    distanceOfClosestBlob = std::numeric_limits<float>::infinity();
    
    for (auto & blob : blobs) {
        // millimeters to meters
        float x = blob.centroid.x * 0.001;
        float y = blob.centroid.y * 0.001;
        
        if (polyline.inside(x, y)) {
            isBlobInside = true;
            float distance = centroid.distance(ofPoint(x, y));
            
            if (distance < distanceOfClosestBlob) {
                distanceOfClosestBlob = distance;
            }
        }
    }
}

void Filter::setTranslation(ofPoint _translation) {
    translation = _translation;
}

void Filter::setSpace(Space & _space) {
    space = _space;
    scale = space.width / (space.areaSize * 1000.0);
}

ofPoint Filter::convertCoordinateToScreenPoint(ofPoint coordinate) {
    return coordinate * 1000.0 * scale + space.origin + translation;
}

ofPoint Filter::convertScreenPointToCoordinate(ofPoint screenPoint) {
    return (screenPoint - space.origin - translation) / scale * 0.001;
}

void Filter::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < positions.size(); i++) {
        ofPoint screenPoint = convertCoordinateToScreenPoint(positions[i]);

        if(mousePoint.distance(screenPoint) <= positions[i].halfSize) {
            positions[i].isMouseOver = true;
        } else {
            positions[i].isMouseOver = false;
        }
    }
    
    ofPoint screenCentroid = convertCoordinateToScreenPoint(centroid);
    
    if (mousePoint.distance(screenCentroid) < centroid.halfSize) {
        centroid.isMouseOver = true;
    } else {
        centroid.isMouseOver = false;
    }
}

void Filter::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < positions.size(); i++) {
        ofPoint screenPoint = convertCoordinateToScreenPoint(positions[i]);

        if(mousePoint.distance(screenPoint) <= positions[i].halfSize) {
            positions[i].isMouseClicked = true;
        } else {
            positions[i].isMouseClicked = false;
        }
    }
    
    ofPoint screenCentroid = convertCoordinateToScreenPoint(centroid);
    
    if (mousePoint.distance(screenCentroid) < centroid.halfSize) {
        centroid.isMouseClicked = true;
    } else {
        centroid.isMouseClicked = false;
    }
}

void Filter::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < positions.size(); i++) {
        if(positions[i].isMouseClicked) {
            ofPoint coordinate = convertScreenPointToCoordinate(mousePoint);
            points[i] = coordinate;
        }
    }
    
    if (centroid.isMouseClicked) {
        ofPoint coordinate = convertScreenPointToCoordinate(mousePoint);
        translatePointsByCentroid(coordinate);
    }
}

void Filter::translatePointsByCentroid(ofPoint _centroid) {
    ofPoint difference = _centroid - centroid;

    for (auto & point : points) {
        point += difference;
    }
}

void Filter::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    for(int i = 0; i < positions.size(); i++) {
        if (positions[i].isMouseClicked) {
           positions[i].isMouseClicked = false;
        }
    }
    
    if (centroid.isMouseClicked) {
        centroid.isMouseClicked = false;
        centroid.isMouseOver = false;
    }
}

void Filter::onKeyPressed(ofKeyEventArgs& keyArgs) {
    if (centroid.isMouseOver) {
        if (keyArgs.key == 102) mask = !mask;
        if (keyArgs.key == 116) isActive = !isActive;
    }
}
