//
//  QuadFilter.hpp
//  meatbags
//

#ifndef QuadFilter_hpp
#define QuadFilter_hpp

#include <stdio.h>
#include "Filter.hpp"

class QuadFilter : public Filter {
public:
    QuadFilter();
    
    void update();
    void updateNormalization();
    virtual void reset() override;
    
    ofPoint normalizeCoordinate(float x, float y);
    ofPoint normalizeSize(float x, float y, float width, float height);
    void updateHomography();

    cv::Mat homography;
    vector<cv::Point2f> quad;
    vector<cv::Point2f> normalizedQuad;
};

#endif /* QuadFilter_hpp */
