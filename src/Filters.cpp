//
//  Filters.cpp
//

#include "Filters.hpp"

Filters::Filters() {
}

void Filters::update() {
    for (auto & filter : filters) {
        filter->update();
    }
}

void Filters::checkBlobs(vector<Blob> & blobs) {
    for (auto & filter : filters) {
        filter->checkBlobs(blobs);
    }
}

void Filters::setSpace(Space & _space) {    
    for (auto & filter : filters) {
        filter->setSpace(_space);
    }
}

void Filters::setTranslation(ofPoint _translation) {
    for (auto & filter : filters) {
        filter->setTranslation(_translation);
    }
}

void Filters::addFilter(Filter* filter) {
    filters.push_back(filter);
}

void Filters::removeFilter() {
    filters.pop_back();
}
