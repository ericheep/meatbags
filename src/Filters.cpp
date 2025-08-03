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

void Filter::translatePointsByCentroid(ofPoint _centroid) {
    ofPoint difference = _centroid - centroid;

    for (auto & point : points) {
        point += difference;
    }
}

bool Filters::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    for (auto& filter : filters) {
        if (filter->onMouseMoved(mouseArgs)) return true;
    }
    return false;
}

bool Filters::onMousePressed(ofMouseEventArgs& mouseArgs) {
    for (auto& filter : filters) {
        if(filter->onMousePressed(mouseArgs)) return true;
    }
    return false;
}

bool Filters::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    for (auto& filter : filters) {
        if(filter->onMouseDragged(mouseArgs)) return true;
    }
    return false;
}

bool Filters::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    for (auto& filter : filters) {
        if(filter->onMouseReleased(mouseArgs)) return true;
    }
    return false;
}

bool Filters::onKeyPressed(ofKeyEventArgs& keyArgs) {
    for (auto& filter : filters) {
        if(filter->onKeyPressed(keyArgs)) return true;
    }
    return false;
}
