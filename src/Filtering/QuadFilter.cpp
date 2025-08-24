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

void QuadFilter::reset() {
    update();
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
    normalizedCoordinate.y = outputVec[0].y;

    return normalizedCoordinate;
}

ofPoint QuadFilter::normalizeSize(float x, float y, float width, float height) {
    vector<cv::Point2f> inputCorners = {
    	cv::Point2f(x, y), // top-left
    	cv::Point2f(x + width, y), // top-right
    	cv::Point2f(x, y + height), // bottom-left
    	cv::Point2f(x + width, y + height) // bottom-right
    };

    vector<cv::Point2f> outputCorners;
    cv::perspectiveTransform(inputCorners, outputCorners, homography);
    
    // Calculate width and height from transformed corners
    float normalizedWidth = cv::norm(outputCorners[1] - outputCorners[0]); // top edge
    float normalizedHeight = cv::norm(outputCorners[2] - outputCorners[0]); // left edge

    return ofVec2f(normalizedWidth, normalizedHeight);
}
