//
//  Filters.cpp
//

#include "Filters.hpp"

Filters::Filters() {
}

void Filters::update() {
    for (auto& filter : filters) {
        filter->update();
    }
}

void Filters::setSpace(Space & _space) {    
    for (auto& filter : filters) {
        filter->setSpace(_space);
    }
}

void Filters::addFilter(Filter* filter) {
    filters.push_back(filter);
}

void Filters::removeFilter() {
    filters.pop_back();
}
