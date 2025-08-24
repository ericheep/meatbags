//
//  EllipseFilter.cpp
//  meatbags
//

#include "EllipseFilter.hpp"

EllipseFilter::EllipseFilter() {
}

void EllipseFilter::updateNormalization() {
}

ofPoint EllipseFilter::getPointOnEllipse(int index) {
    ofPoint center;
    for (auto& p : draggablePoints) center += p;
    center /= 4.0;

    float maxDist = 0;
    int i1 = 0, i2 = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            float d = draggablePoints[i].distance(draggablePoints[j]);
            if (d > maxDist) {
                maxDist = d;
                i1 = i;
                i2 = j;
            }
        }
    }

    ofPoint major1 = draggablePoints[i1];
    ofPoint major2 = draggablePoints[i2];

    std::vector<int> remaining;
    for (int i = 0; i < 4; ++i) {
        if (i != i1 && i != i2) remaining.push_back(i);
    }
    ofPoint minor1 = draggablePoints[remaining[0]];
    ofPoint minor2 = draggablePoints[remaining[1]];
    
    float theta = ofMap(index, 0, ellipseResolution, 0, TWO_PI);
    
    ofPoint majorVec = major2 - major1;
    
    float rotation = atan2(majorVec.y, majorVec.x);
    float a = major1.distance(major2) * 0.5;
    float b = minor1.distance(minor2) * 0.5;

    float ct = cos(theta);
    float st = sin(theta);
    float cp = cos(rotation);
    float sp = sin(rotation);
    
    float x = center.x + a * ct * cp - b * st * sp;
    float y = center.y + a * ct * sp + b * st * cp;

    return ofPoint(x, y);
}

void EllipseFilter::update() {
    polyline.clear();
    for (int i = 0; i < ellipseResolution; i++) {
        polyline.addVertex(getPointOnEllipse(i));
    }
    polyline.close();
    
    Filter::update();

    for (int i = 0; i < draggablePoints.size(); i++) {
        if (draggablePoints[i].isMouseClicked) {
            updateDraggablePoints(i);
        }
    }
}

void EllipseFilter::reset() {
    updateDraggablePoints(0);
}

void EllipseFilter::updateDraggablePoints(int anchorIndex) {
    int oppositeIndex = (anchorIndex + 2) % 4;
    int nextIndex = (anchorIndex + 1) % 4;
    int lastIndex = (anchorIndex + 3) % 4;
    
    ofPoint difference = draggablePoints[anchorIndex] - centroid;
    float theta = atan2(difference.y, difference.x);

    float oppositeRadius = centroid.distance(ofPoint(anchorPoints[anchorIndex]));
    float oppositeTheta = theta + PI;
    float x = cos(oppositeTheta) * oppositeRadius;
    float y = sin(oppositeTheta) * oppositeRadius;
    anchorPoints[oppositeIndex] = ofPoint(x, y) + centroid;
    
    float nextRadius = centroid.distance(ofPoint(anchorPoints[nextIndex]));
    float nextTheta = theta - HALF_PI;
    x = cos(nextTheta) * nextRadius;
    y = sin(nextTheta) * nextRadius;
    anchorPoints[nextIndex] = ofPoint(x, y) + centroid;
    
    float lastRadius = centroid.distance(ofPoint(anchorPoints[lastIndex]));
    float lastTheta = theta + HALF_PI;
    x = cos(lastTheta) * lastRadius;
    y = sin(lastTheta) * lastRadius;
    anchorPoints[lastIndex] = ofPoint(x, y) + centroid;
}

ofPoint EllipseFilter::normalizeCoordinate(float x, float y) {
    ofPoint inputPoint(x, y);
    
    ofPoint center;
    for (auto& p : draggablePoints) center += p;
    center /= 4.0;
    
    float maxDist = 0;
    int i1 = 0, i2 = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            float d = draggablePoints[i].distance(draggablePoints[j]);
            if (d > maxDist) {
                maxDist = d;
                i1 = i;
                i2 = j;
            }
        }
    }
    
    ofPoint major1 = draggablePoints[i1];
    ofPoint major2 = draggablePoints[i2];
    
    std::vector<int> remaining;
    for (int i = 0; i < 4; ++i) {
        if (i != i1 && i != i2) remaining.push_back(i);
    }
    ofPoint minor1 = draggablePoints[remaining[0]];
    ofPoint minor2 = draggablePoints[remaining[1]];
    
    ofPoint majorVec = major2 - major1;
    float rotation = atan2(majorVec.y, majorVec.x);
    float a = major1.distance(major2) * 0.5;
    float b = minor1.distance(minor2) * 0.5;
    
    ofPoint translatedPoint = inputPoint - center;
    
    float cp = cos(-rotation);
    float sp = sin(-rotation);
    float rotatedX = translatedPoint.x * cp - translatedPoint.y * sp;
    float rotatedY = translatedPoint.x * sp + translatedPoint.y * cp;
    
    ofPoint normalizedCoordinate(-rotatedX / a, rotatedY / b);
    
    return normalizedCoordinate;
}

ofPoint EllipseFilter::normalizeSize(float x, float y, float width, float height) {
    ofPoint center;
    for (auto& p : draggablePoints) center += p;
    center /= 4.0;

    float maxDist = 0;
    int i1 = 0, i2 = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            float d = draggablePoints[i].distance(draggablePoints[j]);
            if (d > maxDist) {
                maxDist = d;
                i1 = i;
                i2 = j;
            }
        }
    }

    ofPoint major1 = draggablePoints[i1];
    ofPoint major2 = draggablePoints[i2];

    std::vector<int> remaining;
    for (int i = 0; i < 4; ++i) {
        if (i != i1 && i != i2) remaining.push_back(i);
    }
    ofPoint minor1 = draggablePoints[remaining[0]];
    ofPoint minor2 = draggablePoints[remaining[1]];

    ofPoint majorVec = major2 - major1;
    float rotation = atan2(majorVec.y, majorVec.x);
    float a = major1.distance(major2) * 0.5;
    float b = minor1.distance(minor2) * 0.5;

    // Create width and height vectors
    ofPoint widthVec(width, 0);
    ofPoint heightVec(0, height);

    // Apply inverse rotation to align with ellipse coordinate system
    float cp = cos(-rotation);
    float sp = sin(-rotation);

    float normalizedWidth = (widthVec.x * cp - widthVec.y * sp) / a;
    float normalizedHeight = (heightVec.x * sp + heightVec.y * cp) / b;

    return ofPoint(abs(normalizedWidth), abs(normalizedHeight));
}
