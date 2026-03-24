//
//  MeatbagsPanel.hpp
//  meatbags

#ifndef MeatbagsPanel_hpp
#define MeatbagsPanel_hpp

#include "ofMain.h"
#include "Meatbags.hpp"
#include "DBSCANClusterer.hpp"
#include "EuclideanClusterer.hpp"
#include <vector>
#include <functional>

class MeatbagsPanel {
public:
	MeatbagsPanel();

	void draw(const std::vector<Meatbags*>& meatbags);
	int  totalHeight(int numInstances);

	void saveTo(ofJson& config);
	void loadFrom(ofJson& config);
	bool onMousePressed(ofMouseEventArgs& args, const std::vector<Meatbags*>& meatbags);
	void onMouseDragged(ofMouseEventArgs& args, const std::vector<Meatbags*>& meatbags);
	bool onMouseReleased(ofMouseEventArgs& args);
	bool onKeyPressed(ofKeyEventArgs& args, const std::vector<Meatbags*>& meatbags);
	void onMouseMoved(ofMouseEventArgs& args, const std::vector<Meatbags*>& meatbags);

	int x, y, width;

private:
	// -------------------------------------------------------------------------
	// Layout
	// -------------------------------------------------------------------------
	ofRectangle panelHeaderRect();
	ofRectangle instanceHeaderRect(int i);
	ofRectangle clustererRowRect(int i);
	ofRectangle clustererItemRect(int i, int itemIndex);
	ofRectangle paramRowRect(int i, int p);
	ofRectangle sliderRect(int i, int p);
	ofRectangle numberBoxRect(int i, int p);
	int instanceYOffset(int i);
	int instanceHeight();

	// -------------------------------------------------------------------------
	// Draw helpers
	// -------------------------------------------------------------------------
	void drawHeader();
	void drawInstance(int i, Meatbags* m);
	void drawInstanceHeader(int i);
	void drawClustererDropdown(int i, Meatbags* m);
	void drawParamRow(int i, int p, Meatbags* m);
	void drawSlider(const ofRectangle& rect, float value, float min, float max, bool active);
	void drawNumberBox(const ofRectangle& rect, float value, bool editing);

	// -------------------------------------------------------------------------
	// Param helpers
	// -------------------------------------------------------------------------
	struct ParamInfo {
		std::string label;
		float value, min, max;
	};
	ParamInfo getParamInfo(int p, Meatbags* m);
	void      setParamValue(int p, float value, Meatbags* m);

	static const std::vector<std::string>& clustererNames();

	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------
	bool isCollapsed;

	// per-instance clusterer dropdown open state
	std::vector<bool>   clustererDropdownOpen;
	std::vector<string> clustererHoveredItem;

	// slider drag
	bool isDragging;
	int  dragInstance, dragParam;

	// number box editing
	bool        isEditing;
	int         editInstance, editParam;
	std::string editText;

	// -------------------------------------------------------------------------
	// Style
	// -------------------------------------------------------------------------
	ofColor backgroundColor;
	ofColor headerColor;
	ofColor headerTextColor;
	ofColor instanceHeaderColor;
	ofColor rowColor;
	ofColor rowAltColor;
	ofColor dropdownRowColor;
	ofColor dropdownItemColor;
	ofColor dropdownItemHoverColor;
	ofColor sliderFillColor;
	ofColor sliderTrackColor;
	ofColor numberBoxColor;
	ofColor numberBoxActiveColor;
	ofColor textColor;
	ofColor dimTextColor;
	ofColor accentColor;

	int headerHeight;
	int instanceHeaderHeight;
	int rowHeight;
	int padding;
	int indent;
	float numberBoxRatio;
};

#endif /* MeatbagsPanel_hpp */
