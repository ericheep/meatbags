//
//  Filters.hpp
//

#ifndef Filters_hpp
#define Filters_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Filter.hpp"

class Filters {
public:
    Filters();
    
    void update();
    void addFilter(Filter* filter);
    void removeFilter();
    
    void setSpace(Space & space);
    
    vector <Filter *> filters;
};

#endif /* Filters_hpp */
