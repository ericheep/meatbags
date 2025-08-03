//
//  Filters.hpp
//

#ifndef Filters_hpp
#define Filters_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Filter.hpp"
#include "Blob.hpp"

class Filters {
public:
    Filters();
    
    void update();
    void checkBlobs(vector<Blob> & blobs);
    void addFilter(Filter* filter);
    void removeFilter();
    
    void setTranslation(ofPoint translation);
    void setSpace(Space & space);
    
    bool onMouseMoved(ofMouseEventArgs & mouseArgs);
    bool onMousePressed(ofMouseEventArgs & mouseArgs);
    bool onMouseDragged(ofMouseEventArgs & mouseArgs);
    bool onMouseReleased(ofMouseEventArgs & mouseArgs);
    bool onKeyPressed(ofKeyEventArgs & keyArgs);
    
    vector <Filter *> filters;
};

#endif /* Filters_hpp */
