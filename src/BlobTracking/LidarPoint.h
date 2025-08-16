//
//  LidarPoint.h
//  meatbags
//

#pragma once
#include "ofMain.h"

struct LidarPoint {
    ofPoint coordinate;
    ofColor color;
    bool isInFilter;
};
