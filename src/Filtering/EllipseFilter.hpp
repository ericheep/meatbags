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
    
    virtual void update() override;
    virtual void updateNormalization() override;
    virtual void reset() override;
    virtual ofPoint normalizeCoordinate(float x, float y) override;
    virtual ofPoint normalizeSize(float x, float y, float width, float height) override;

    void updateDraggablePoints(int anchorIndex);

    ofPoint getPointOnEllipse(int index);
    float xRadius, yRadius, rotation;
    int ellipseResolution = 200;
};

#endif /* EllipseFilter_hpp */
