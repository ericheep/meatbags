//
//  SensorPanel.hpp
//  meatbags

#ifndef SensorPanel_hpp
#define SensorPanel_hpp

#include "ofMain.h"
#include "Sensor.hpp"
#include "Hokuyo.hpp"
#include "OrbbecPulsar.hpp"
#include <vector>
#include <functional>

class SensorPanel {
public:
	SensorPanel();

	void draw(const std::vector<Sensor*>& sensors);

	bool onMousePressed(ofMouseEventArgs& args, const std::vector<Sensor*>& sensors);
	void onMouseDragged(ofMouseEventArgs& args, const std::vector<Sensor*>& sensors);
	bool onMouseReleased(ofMouseEventArgs& args);
	bool onKeyPressed(ofKeyEventArgs& args, const std::vector<Sensor*>& sensors);
	void onMouseMoved(ofMouseEventArgs& args, const std::vector<Sensor*>& sensors);
	bool onMouseScrolled(ofMouseEventArgs& args, const std::vector<Sensor*>& sensors);

	void saveTo(ofJson& config);
	void loadFrom(ofJson& config);

	std::function<void(int, string)> onSensorTypeChanged;

	int   x, y, width;
	float scrollY;

private:
	// -------------------------------------------------------------------------
	// Row definitions
	// -------------------------------------------------------------------------
	enum class RowType {
		TypeDropdown,
		IPField,
		WhichMeatbag,
		Position,
		Rotation,
		MirrorAngles,
		ShowInfo,
		MotorSpeed,
		FogMode,
		Standby
	};

	std::vector<RowType> rowsForSensor(Sensor* s);
	int numRows(Sensor* s);
	bool isOrbbec(Sensor* s);

	// -------------------------------------------------------------------------
	// Layout
	// -------------------------------------------------------------------------
	ofRectangle panelHeaderRect();
	ofRectangle scrollbarTrackRect();
	ofRectangle scrollbarThumbRect(const std::vector<Sensor*>& sensors);
	int instanceYOffset(int i, const std::vector<Sensor*>& sensors);
	int instanceHeight(int i, Sensor* s);
	int totalContentHeight(const std::vector<Sensor*>& sensors);
	int availableHeight();
	bool isScrollable(const std::vector<Sensor*>& sensors);
	void clampScroll(const std::vector<Sensor*>& sensors);

	// -------------------------------------------------------------------------
	// Draw
	// -------------------------------------------------------------------------
	void drawHeader();
	void drawScrollbar(const std::vector<Sensor*>& sensors);
	void drawInstance(int i, Sensor* s, const std::vector<Sensor*>& sensors);

	// -------------------------------------------------------------------------
	// Param helpers
	// -------------------------------------------------------------------------
	struct SliderInfo { string label; float value, min, max; bool isInt; };
	SliderInfo getSliderInfo(RowType type, Sensor* s);
	void       setSliderValue(RowType type, float value, Sensor* s);

	bool getCheckboxValue(RowType type, Sensor* s);
	void setCheckboxValue(RowType type, bool value, Sensor* s);

	static const std::vector<std::string>& sensorTypeNames();
	static string rowLabel(RowType type);

	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------
	bool isCollapsed;
	std::vector<bool>   instanceCollapsed;
	std::vector<bool>   typeDropdownOpen;
	std::vector<string> typeHoveredItem;

	// scroll
	int   bottomMargin;
	bool  isScrollbarDragging;
	float scrollbarDragStartY;
	float scrollbarDragStartScrollY;

	// slider drag
	bool isDragging;
	int  dragInstance, dragRowIndex;

	// float number box editing
	bool        isEditingFloat;
	int         editFloatInstance, editFloatRow;
	std::string editFloatText;

	// pos box editing (x=0, y=1)
	bool        isEditingPos;
	int         editPosInstance, editPosComponent;
	std::string editPosText;

	// IP field editing
	bool        isEditingIP;
	int         editIPInstance;
	std::string editIPText;

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
	ofColor ipBoxColor;
	ofColor ipBoxActiveColor;
	ofColor controlRowColor;
	ofColor scrollbarColor;
	ofColor scrollbarThumbColor;
	ofColor textColor;
	ofColor dimTextColor;
	ofColor accentColor;
	ofColor connectedColor;
	ofColor disconnectedColor;

	int   headerHeight;
	int   instanceHeaderHeight;
	int   rowHeight;
	int   checkboxSize;
	int   padding;
	int   indent;
	int   numberBoxW;
	int   scrollbarWidth;
};

#endif /* SensorPanel_hpp */
