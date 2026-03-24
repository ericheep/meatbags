//
//  FilterPanel.hpp
//  meatbags

#ifndef FilterPanel_hpp
#define FilterPanel_hpp

#include "ofMain.h"
#include "Filter.hpp"
#include "QuadFilter.hpp"
#include "EllipseFilter.hpp"
#include <vector>
#include <functional>

class FilterPanel {
public:
	FilterPanel();

	void draw(const std::vector<Filter*>& filters);

	bool onMousePressed(ofMouseEventArgs& args, const std::vector<Filter*>& filters);
	void onMouseMoved(ofMouseEventArgs& args, const std::vector<Filter*>& filters);
	bool onMouseScrolled(ofMouseEventArgs& args, const std::vector<Filter*>& filters);
	bool onMouseReleased(ofMouseEventArgs& args);

	void saveTo(ofJson& config);
	void loadFrom(ofJson& config);

	int totalHeight(int n);
	int totalContentHeight(const std::vector<Filter*>& filters);

	std::function<void(int, string)> onFilterTypeChanged;

	int   x, y, width;
	float scrollY;

private:
	// -------------------------------------------------------------------------
	// Layout
	// -------------------------------------------------------------------------
	ofRectangle panelHeaderRect();
	ofRectangle scrollbarTrackRect();
	ofRectangle scrollbarThumbRect(const std::vector<Filter*>& filters);
	int instanceYOffset(int i, const std::vector<Filter*>& filters);
	int instanceHeight(int i);
	int availableHeight();
	bool isScrollable(const std::vector<Filter*>& filters);
	void clampScroll(const std::vector<Filter*>& filters);

	// -------------------------------------------------------------------------
	// Draw
	// -------------------------------------------------------------------------
	void drawHeader();
	void drawScrollbar(const std::vector<Filter*>& filters);
	void drawInstance(int i, Filter* f, const std::vector<Filter*>& filters);

	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------
	bool isCollapsed;
	std::vector<bool>   instanceCollapsed;
	std::vector<bool>   typeDropdownOpen;
	std::vector<string> typeHoveredItem;

	bool  isScrollbarDragging;
	float scrollbarDragStartY;
	float scrollbarDragStartScrollY;

	static const std::vector<std::string>& filterTypeNames();
	string currentTypeName(Filter* f);

	// -------------------------------------------------------------------------
	// Style
	// -------------------------------------------------------------------------
	ofColor backgroundColor;
	ofColor headerColor;
	ofColor headerTextColor;
	ofColor instanceHeaderColor;
	ofColor rowColor;
	ofColor rowAltColor;
	ofColor controlRowColor;
	ofColor dropdownItemColor;
	ofColor dropdownItemHoverColor;
	ofColor scrollbarColor;
	ofColor scrollbarThumbColor;
	ofColor textColor;
	ofColor dimTextColor;
	ofColor accentColor;

	int headerHeight;
	int instanceHeaderHeight;
	int rowHeight;
	int checkboxSize;
	int padding;
	int indent;
	int bottomMargin;
	int scrollbarWidth;
};

#endif /* FilterPanel_hpp */
