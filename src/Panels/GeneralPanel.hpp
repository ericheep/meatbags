//
//  GeneralPanel.hpp
//  meatbags

#ifndef GeneralPanel_hpp
#define GeneralPanel_hpp

#include "ofMain.h"
#include "InterfaceSelector.hpp"
#include <functional>

class GeneralPanel {
public:
	GeneralPanel();

	void setup();
	void draw();

	bool onMousePressed(ofMouseEventArgs& args);
	void onMouseMoved(ofMouseEventArgs& args);

	void saveTo(ofJson& config);
	void loadFrom(ofJson& config);

	int totalHeight();

	std::function<void(string, string)> onInterfaceChanged;
	std::function<void(bool)>           onHeadlessModeChanged;

	int x, y, width;

private:
	void drawHeader();
	void drawInterfaceDropdown();
	void drawHeadlessToggle();

	void selectInterface(const string& selection);

	ofRectangle headerRect();
	ofRectangle dropdownRowRect();
	ofRectangle dropdownItemRect(int index);
	ofRectangle checkboxRect();
	ofRectangle toggleRowRect();

	InterfaceSelector   interfaceSelector;
	string              selectedInterface;
	bool                dropdownOpen;
	bool                isCollapsed;
	bool                headlessMode;
	string              hoveredItem;

	ofColor backgroundColor;
	ofColor headerColor;
	ofColor headerTextColor;
	ofColor rowColor;
	ofColor rowHoverColor;
	ofColor textColor;
	ofColor dimTextColor;
	ofColor checkboxColor;

	int headerHeight;
	int rowHeight;
	int checkboxSize;
	int padding;
};

#endif /* GeneralPanel_hpp */
