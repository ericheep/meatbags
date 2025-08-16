//
//  FilterManager.hpp
//  meatbags
//

#ifndef FilterManager_hpp
#define FilterManager_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxDropdown.h"
#include "Filter.hpp"
#include "QuadFilter.hpp"
#include "EllipseFilter.hpp"
#include "Space.h"
#include "Blob.hpp"

enum class FilterType {
    Ellipse,
    Quad
};

class FilterManager {
public:
    FilterManager();
    ~FilterManager();

    void addFilter();
    void addFilter(FilterType type);
    void changeFilterType(int index, FilterType newType);
    void removeFilter();
    void onFilterTypeChanged(string& selectedType);
    void clear();
    
    void update();
    void draw();

    void initialize();
    void saveTo(ofJson& configuration);
    void load(ofJson configuration);
    void loadFilters(int numberFilters, ofJson configuration);
    
    void checkBlobs(std::vector<Blob>& blobs);
    void setSpace(Space& space);
    void setTranslation(ofPoint translation);
    
    bool onMouseMoved(ofMouseEventArgs& mouseArgs);
    bool onMousePressed(ofMouseEventArgs& mouseArgs);
    bool onMouseDragged(ofMouseEventArgs& mouseArgs);
    bool onMouseReleased(ofMouseEventArgs& mouseArgs);
    bool onKeyPressed(ofKeyEventArgs& keyArgs);
    
    vector<Filter*> getFilters();
    void refreshGUIPositions();
private:
    struct FilterEntry {
        unique_ptr<Filter> filter;
        unique_ptr<ofxPanel> gui;
        string nextType;
    };
    
    vector<FilterEntry> filterEntries;
    
    unique_ptr<Filter> createFilterOfType(FilterType type);
    unique_ptr<ofxPanel> createGUIForFilter(Filter* filter, FilterType type);
    
    void setupCommonGUI(ofxPanel* gui, Filter* filter);
    void setupEllipseGUI(ofxPanel* gui, EllipseFilter* filter);
    void setupQuadGUI(ofxPanel* gui, QuadFilter* filter);
    
    FilterType getCurrentFilterType(Filter* filter);
    FilterType stringToFilterType(const std::string& typeStr);
    string filterTypeToString(FilterType type);
    
    void transferFilterState(Filter* oldFilter, Filter* newFilter);
    int guiWidth;
};

#endif /* FilterManager_hpp */
