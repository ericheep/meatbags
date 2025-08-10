//
//  FilterManager.cpp
//  meatbags
//

#include "FilterManager.hpp"

FilterManager::FilterManager() {
    guiWidth = 120;
}

FilterManager::~FilterManager() {
    clear();
}

void FilterManager::clear() {
    for (auto& entry : filterEntries) {
        if (entry.filter) {
            entry.filter->filterTypes.removeListener(this, &FilterManager::onFilterTypeChanged);
        }
    }
    filterEntries.clear();
}

void FilterManager::saveTo(ofJson& config) {
    config["number_filters"] = filterEntries.size();
    for (auto& entry : filterEntries) {
        entry.gui->saveTo(config);
    }
}

void FilterManager::loadFilters(int numberFilters, ofJson config) {
    for (int i = 0; i < numberFilters; i++) {
        addFilter();
    }
    for (int i = 0; i < filterEntries.size(); i++) {
        string filterKey = "filter_" + to_string(i + 1);
        ofJson filterConfig;
        filterConfig[filterKey] = config[filterKey];
        filterEntries[i].gui->loadFrom(filterConfig);
    }
}

void FilterManager::load(ofJson config) {
    if (config.contains("number_filters")) {
        int numberFilters = config["number_filters"];
        loadFilters(numberFilters, config);
    } else {
        addFilter();
    }
}

void FilterManager::addFilter() {
    if (filterEntries.size() < 12) {
        addFilter(FilterType::Quad);
    }
}

void FilterManager::addFilter(FilterType type) {
    auto filter = createFilterOfType(type);
    
    if (filter) filter->index = filterEntries.size() + 1;
    
    auto gui = createGUIForFilter(filter.get(), type);
    filterEntries.push_back({std::move(filter), std::move(gui)});
    refreshGUIPositions();
}

void FilterManager::removeFilter() {
    if (filterEntries.size() > 1) {
        filterEntries.back().filter->filterTypes.removeListener(this, &FilterManager::onFilterTypeChanged);
        filterEntries.pop_back();
    }
}

void FilterManager::changeFilterType(int index, FilterType newType) {
    if (index < 0 || index >= filterEntries.size()) return;

    auto& entry = filterEntries[index];
    
    // Check if it's actually a different type
    FilterType currentType = getCurrentFilterType(entry.filter.get());
    if (currentType == newType) return;
    
    // Store current state BEFORE removing listener
    Filter* oldFilter = entry.filter.get();
    
    // Remove old listener - this is critical to do BEFORE creating new filter
    if (oldFilter) {
        oldFilter->filterTypes.removeListener(this, &FilterManager::onFilterTypeChanged);
    }
    
    // Create new filter
    auto newFilter = createFilterOfType(newType);
    if (!newFilter) return;
    
    // Transfer state from old filter to new filter
    if (oldFilter) {
        transferFilterState(oldFilter, newFilter.get());
    }
    
    // Create new GUI - this will set up the parameters properly
    auto newGUI = createGUIForFilter(newFilter.get(), newType);
    
    // Replace everything at once
    entry.filter = std::move(newFilter);
    entry.gui = std::move(newGUI);
    refreshGUIPositions();
}

void FilterManager::update() {
    for (auto& entry : filterEntries) {
        if (entry.filter) {
            entry.filter->update();
        }
    }
    
    for (int i = 0; i < filterEntries.size(); ++i) {
        auto& entry = filterEntries[i];
        if (!entry.nextType.empty()) {
            std::string type = entry.nextType;
            entry.nextType.clear();
            
            FilterType newFilterType = stringToFilterType(type);
            FilterType currentType = getCurrentFilterType(entry.filter.get());
            
            if (newFilterType != currentType) {
                changeFilterType(i, newFilterType);
            }
        }
    }
}

void FilterManager::transferFilterState(Filter* oldFilter, Filter* newFilter) {
    if (!oldFilter || !newFilter) return;
    
    // Transfer common properties
    newFilter->index = oldFilter->index;
    newFilter->isMask = oldFilter->isMask;
    newFilter->isNormalized = oldFilter->isNormalized;
    
    // Transfer anchor points - these are ofParameter<ofVec2f> objects
    for (int i = 0; i < 4; i++) {
        newFilter->anchorPoints[i] = oldFilter->anchorPoints[i].get();
    }
    
    // Transfer space and translation if they exist
    newFilter->setSpace(oldFilter->space);
    newFilter->setTranslation(oldFilter->translation);
    
    // Make sure the new filter updates its internal state
    newFilter->update();
    newFilter->updateNormalization();
}

FilterType FilterManager::getCurrentFilterType(Filter* filter) {
    if (dynamic_cast<EllipseFilter*>(filter)) {
        return FilterType::Ellipse;
    } else if (dynamic_cast<QuadFilter*>(filter)) {
        return FilterType::Quad;
    }
    return FilterType::Ellipse; // default
}

FilterType FilterManager::stringToFilterType(const std::string& typeStr) {
    if (typeStr == "ellipse") {
        return FilterType::Ellipse;
    } else if (typeStr == "quad") {
        return FilterType::Quad;
    }
    // Add other types as needed
    return FilterType::Ellipse; // default fallback
}

string FilterManager::filterTypeToString(FilterType type) {
    switch (type) {
        case FilterType::Ellipse: return "ellipse";
        case FilterType::Quad: return "quad";
        default: return "ellipse";
    }
}

void FilterManager::draw() {
    for (auto& entry : filterEntries) {
        if (entry.gui) {
            entry.gui->draw();
        }
    }
}

std::vector<Filter*> FilterManager::getFilters() {
    std::vector<Filter*> result;
    for (auto& entry : filterEntries) {
        result.push_back(entry.filter.get());
    }
    return result;
}

std::unique_ptr<Filter> FilterManager::createFilterOfType(FilterType type) {
    switch (type) {
        case FilterType::Ellipse:
            return std::make_unique<EllipseFilter>();
        case FilterType::Quad:
            return std::make_unique<QuadFilter>();
        default:
            return nullptr;
    }
}

std::unique_ptr<ofxPanel> FilterManager::createGUIForFilter(Filter* filter, FilterType type) {
    ofxGuiSetDefaultWidth(guiWidth);
    auto gui = std::make_unique<ofxPanel>();
    gui->setup("filter " + to_string(filter->index));

    setupCommonGUI(gui.get(), filter);

    switch (type) {
        case FilterType::Ellipse:
            setupEllipseGUI(gui.get(), dynamic_cast<EllipseFilter*>(filter));
            break;
        case FilterType::Quad:
            setupQuadGUI(gui.get(), dynamic_cast<QuadFilter*>(filter));
            break;
    }

    return gui;
}

void FilterManager::setupCommonGUI(ofxPanel* gui, Filter* filter) {
    if (!gui || !filter) return;
    
    float centerRatio = float(filterEntries.size()) / 15.0;
    float cx = cos(centerRatio * TWO_PI - HALF_PI) * 2.25;
    float cy = sin(centerRatio * TWO_PI - HALF_PI) * 2.25;
    ofPoint center = ofPoint(cx, cy + 6.0);
    
    FilterType currentType = getCurrentFilterType(filter);
    filter->filterTypes.selectedValue = filterTypeToString(currentType);
    
    ofColor filterColor = ofColor::thistle;
    ofColor backgroundColor = ofColor::snow;
    backgroundColor.a = 210;
    
    gui->setDefaultBackgroundColor(backgroundColor);
    gui->setBackgroundColor(backgroundColor);
    gui->setHeaderBackgroundColor(filterColor);
    gui->setFillColor(filterColor);
    gui->setDefaultFillColor(filterColor);
    gui->add(&filter->filterTypes);
    
    filter->filterTypes.setBackgroundColor(backgroundColor);
    filter->filterTypes.setDefaultFillColor(backgroundColor);
    filter->filterTypes.setFillColor(filterColor);
    filter->filterTypes.setBackgroundColor(backgroundColor);
    filter->filterTypes.setTextColor(ofColor::black);
    filter->filterTypes.setDefaultHeaderBackgroundColor(backgroundColor);
    
    gui->add(filter->isMask.set("mask", false));
    gui->add(filter->isNormalized.set("normalize", true));
    
    ofParameterGroup anchorPointSettings;
    anchorPointSettings.setName("coordinates");
    
    for (int i = 0; i < 4; i++) {
        ofVec2f currentPoint = filter->anchorPoints[i];
        
        if (currentPoint.x == 0 && currentPoint.y == 0) {
            float ratio = float(i) / 4;
            float x = cos(ratio * TWO_PI + HALF_PI * 0.5) * 0.5 + center.x;
            float y = sin(ratio * TWO_PI + HALF_PI * 0.5) * 0.5 + center.y;
            filter->anchorPoints[i] = ofVec2f(x, y);
        }

        anchorPointSettings.add(filter->anchorPoints[i].set("p" + to_string(i),
                                                            filter->anchorPoints[i],
                                                            ofVec2f(-10, -10),
                                                            ofVec2f(10, 10)));
    }
    
    gui->add(anchorPointSettings);
    gui->getGroup("coordinates").minimize();
    
    filter->update();
    filter->updateNormalization();
    filter->filterTypes.addListener(this, &FilterManager::onFilterTypeChanged);
}

void FilterManager::refreshGUIPositions() {
    for (int i = 0; i < filterEntries.size(); ++i) {
        auto& entry = filterEntries[i];
        if (!(entry.gui && entry.filter)) continue;
        
        int guiHeight = 73;
        int margin = 10;

        int xPos = ofGetWidth() - guiWidth - margin - 70;
        int yPos = margin + i * (guiHeight + margin);
        
        entry.gui->setPosition(xPos, yPos);
    }
}

void FilterManager::setupEllipseGUI(ofxPanel* gui, EllipseFilter* filter) {
    if (!filter) return;
}

void FilterManager::setupQuadGUI(ofxPanel* gui, QuadFilter* filter) {
    if (!filter) return;
}

bool FilterManager::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    for (auto& entry : filterEntries) {
        if (entry.filter->onMouseMoved(mouseArgs)) return true;
    }
    return false;
}

bool FilterManager::onMousePressed(ofMouseEventArgs& mouseArgs) {
    for (auto& entry : filterEntries) {
        if (entry.filter->onMousePressed(mouseArgs)) return true;
    }
    return false;
}

bool FilterManager::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    for (auto& entry : filterEntries) {
        if (entry.filter->onMouseDragged(mouseArgs)) return true;
    }
    return false;
}

bool FilterManager::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    for (auto& entry : filterEntries) {
        if (entry.filter->onMouseReleased(mouseArgs)) return true;
    }
    return false;
}

bool FilterManager::onKeyPressed(ofKeyEventArgs& keyArgs) {
    for (auto& entry : filterEntries) {
        if (entry.filter->onKeyPressed(keyArgs)) return true;
    }
    return false;
}

void FilterManager::checkBlobs(std::vector<Blob>& blobs) {
    for (auto& entry : filterEntries) {
        entry.filter->checkBlobs(blobs);
    }
}

void FilterManager::setSpace(Space& space) {
    for (auto& entry : filterEntries) {
        entry.filter->setSpace(space);
    }
}

void FilterManager::setTranslation(ofPoint translation) {
    for (auto& entry : filterEntries) {
        entry.filter->setTranslation(translation);
    }
}

void FilterManager::onFilterTypeChanged(string& selectedType) {
    for (int i = 0; i < filterEntries.size(); ++i) {
        auto& entry = filterEntries[i];
        if (entry.filter) {
            string currentTypeName = filterTypeToString(getCurrentFilterType(entry.filter.get()));
            string dropdownValue = entry.filter->filterTypes.selectedValue;
            
            if (dropdownValue == selectedType && currentTypeName != selectedType) {
                entry.nextType = selectedType;
                break;
            }
        }
    }
}
