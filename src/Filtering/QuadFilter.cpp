//
//  QuadFilter.cpp
//  meatbags
//

#include "QuadFilter.hpp"

QuadFilter::QuadFilter() {
    normalizedQuad = {
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f }
    };
    
    quad.resize(4);
}

void QuadFilter::updateNormalization() {
    updateHomography();
}

void QuadFilter::update() {
    polyline.clear();
    for (auto anchorPoint : anchorPoints) {
        polyline.addVertex(ofPoint(anchorPoint));
    }
    polyline.close();
    
    Filter::update();
}

void QuadFilter::updateHomography() {
    for(int i = 0; i < draggablePoints.size(); i++) {
        quad[i].x = draggablePoints[i].x;
        quad[i].y = draggablePoints[i].y;
    }
    
    homography = cv::getPerspectiveTransform(quad, normalizedQuad);
}

ofPoint QuadFilter::normalizeCoordinate(float x, float y) {
    cv::Point2f inputPoint(x, y);
    vector<cv::Point2f> inputVec = { inputPoint }, outputVec;
    cv::perspectiveTransform(inputVec, outputVec, homography);
    
    ofPoint normalizedCoordinate;
    normalizedCoordinate.x = outputVec[0].x;
    normalizedCoordinate.x = outputVec[0].y;

    return normalizedCoordinate;
}
