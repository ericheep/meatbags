//
//  DraggablePoint.hpp
//

#ifndef DraggablePoint_hpp
#define DraggablePoint_hpp

#include <stdio.h>
#include "ofMain.h"

class DraggablePoint : public ofPoint {
public:
    DraggablePoint();
    
    float size;
    float halfSize;
    bool isMouseOver;
    bool isMouseClicked;
private:
};


#endif /* DraggablePoint_hpp */
