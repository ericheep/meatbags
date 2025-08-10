//
//  EllipseFilter.hpp
//  meatbags
//

#ifndef EllipseFilter_hpp
#define EllipseFilter_hpp

#include <stdio.h>
#include "Filter.hpp"
#include "math.h"

class EllipseFilter : public Filter {
public:
    EllipseFilter();
    
    void update();
    void updateNormalization();
    void updateDraggablePoints(int anchorIndex);
    
    ofPoint getPointOnEllipse(int index);
    ofPoint normalizeCoordinate(float x, float y);
    float xRadius, yRadius, rotation;
    int ellipseResolution = 200;
};

#endif /* EllipseFilter_hpp */
