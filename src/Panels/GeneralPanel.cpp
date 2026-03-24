//
//  GeneralPanel.cpp
//  meatbags

#include "GeneralPanel.hpp"

GeneralPanel::GeneralPanel() {
	x = 0;
	y = 0;
	width = 220;

	dropdownOpen  = false;
	isCollapsed   = false;
	headlessMode  = false;
	hoveredItem   = "";

	headerHeight  = 20;
	rowHeight     = 18;
	checkboxSize  = 10;
	padding       = 6;

	backgroundColor = ofColor(0, 0, 0, 220);
	headerColor     = ofColor::thistle;
	headerTextColor = ofColor::black;
	rowColor        = ofColor(20, 20, 20, 240);
	rowHoverColor   = ofColor(60, 60, 60, 240);
	textColor       = ofColor::white;
	dimTextColor    = ofColor(180, 180, 180);
	checkboxColor   = ofColor::thistle;
}

void GeneralPanel::setup() {
	interfaceSelector.listInterfaces();

	if (!interfaceSelector.interfacesStrings.empty()) {
		selectInterface(interfaceSelector.interfacesStrings[0]);
	}
}

// -----------------------------------------------------------------------------
// Layout
// -----------------------------------------------------------------------------

int GeneralPanel::totalHeight() {
	if (isCollapsed) return headerHeight;
	int h = headerHeight + rowHeight;
	if (dropdownOpen) h += interfaceSelector.interfacesStrings.size() * rowHeight;
	h += rowHeight;
	return h;
}

ofRectangle GeneralPanel::headerRect() {
	return ofRectangle(x, y, width, headerHeight);
}

ofRectangle GeneralPanel::dropdownRowRect() {
	return ofRectangle(x, y + headerHeight, width, rowHeight);
}

ofRectangle GeneralPanel::dropdownItemRect(int index) {
	int itemY = y + headerHeight + rowHeight + index * rowHeight;
	return ofRectangle(x, itemY, width, rowHeight);
}

ofRectangle GeneralPanel::toggleRowRect() {
	int rowY = y + headerHeight + rowHeight;
	if (dropdownOpen) rowY += interfaceSelector.interfacesStrings.size() * rowHeight;
	return ofRectangle(x, rowY, width, rowHeight);
}

ofRectangle GeneralPanel::checkboxRect() {
	ofRectangle row = toggleRowRect();
	float cx = row.x + padding;
	float cy = row.y + (rowHeight - checkboxSize) * 0.5f;
	return ofRectangle(cx, cy, checkboxSize, checkboxSize);
}

// -----------------------------------------------------------------------------
// Draw
// -----------------------------------------------------------------------------

void GeneralPanel::draw() {
	ofFill();
	ofSetColor(backgroundColor);
	ofDrawRectangle(x, y, width, totalHeight());

	drawHeader();
	if (isCollapsed) return;
	drawInterfaceDropdown();
	drawHeadlessToggle();
}

void GeneralPanel::drawHeader() {
	ofFill();
	ofSetColor(headerColor);
	ofDrawRectangle(headerRect());
	ofSetColor(headerTextColor);
	ofDrawBitmapString("general", x + padding, y + 14);
	// +/- right-aligned
	string indicator = isCollapsed ? "+" : "-";
	float indW = indicator.length() * 8;
	ofDrawBitmapString(indicator, x + width - indW - padding, y + 14);
}

void GeneralPanel::drawInterfaceDropdown() {
	ofRectangle row = dropdownRowRect();
	float rowRight  = (float)(x + width);

	ofFill();
	ofSetColor(rowColor);
	ofDrawRectangle(row);
	ofSetColor(ofColor(80, 80, 80));
	ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());

	ofSetColor(textColor);
	ofDrawBitmapString("iface", row.x + padding, row.y + 13);

	string display = selectedInterface.empty() ? "-" : selectedInterface;
	float valWidth = display.length() * 8;
	ofSetColor(dimTextColor);
	ofDrawBitmapString(display, rowRight - valWidth - padding, row.y + 13);

	if (dropdownOpen) {
		for (int i = 0; i < interfaceSelector.interfacesStrings.size(); i++) {
			ofRectangle itemRow = dropdownItemRect(i);
			string item = interfaceSelector.interfacesStrings[i];

			bool hovered = (hoveredItem == item);
			ofFill();
			ofSetColor(hovered ? rowHoverColor : ofColor(10, 10, 10, 240));
			ofDrawRectangle(itemRow);

			if (item == selectedInterface) {
				ofSetColor(checkboxColor);
				ofDrawBitmapString("*", itemRow.x + padding, itemRow.y + 13);
			}

			ofSetColor(hovered ? textColor : dimTextColor);
			ofDrawBitmapString(item, itemRow.x + padding + 12, itemRow.y + 13);

			ofSetColor(ofColor(50, 50, 50));
			ofDrawLine(itemRow.x, itemRow.getBottom(), itemRow.getRight(), itemRow.getBottom());
		}
	}
}

void GeneralPanel::drawHeadlessToggle() {
	ofRectangle row   = toggleRowRect();
	ofRectangle check = checkboxRect();

	ofFill();
	ofSetColor(rowColor);
	ofDrawRectangle(row);

	ofNoFill();
	ofSetColor(checkboxColor);
	ofDrawRectangle(check);

	if (headlessMode) {
		ofFill();
		ofSetColor(checkboxColor);
		ofRectangle inner = check;
		inner.scaleFromCenter(0.6f);
		ofDrawRectangle(inner);
	}

	ofSetColor(textColor);
	ofDrawBitmapString("headless mode", check.getRight() + padding, row.y + 13);
}

// -----------------------------------------------------------------------------
// Mouse
// -----------------------------------------------------------------------------

bool GeneralPanel::onMousePressed(ofMouseEventArgs& args) {
	ofPoint mouse(args.x, args.y);

	if (headerRect().inside(mouse)) {
		isCollapsed  = !isCollapsed;
		dropdownOpen = false;
		return true;
	}

	if (isCollapsed) return false;

	if (dropdownRowRect().inside(mouse)) {
		dropdownOpen = !dropdownOpen;
		return true;
	}

	if (dropdownOpen) {
		for (int i = 0; i < interfaceSelector.interfacesStrings.size(); i++) {
			if (dropdownItemRect(i).inside(mouse)) {
				selectInterface(interfaceSelector.interfacesStrings[i]);
				dropdownOpen = false;
				return true;
			}
		}
		dropdownOpen = false;
		return true;
	}

	if (checkboxRect().inside(mouse)) {
		headlessMode = !headlessMode;
		if (onHeadlessModeChanged) onHeadlessModeChanged(headlessMode);
		return true;
	}

	return false;
}

void GeneralPanel::onMouseMoved(ofMouseEventArgs& args) {
	ofPoint mouse(args.x, args.y);
	hoveredItem = "";

	if (dropdownOpen) {
		for (int i = 0; i < interfaceSelector.interfacesStrings.size(); i++) {
			if (dropdownItemRect(i).inside(mouse)) {
				hoveredItem = interfaceSelector.interfacesStrings[i];
				break;
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

void GeneralPanel::selectInterface(const string& selection) {
	selectedInterface = selection;
	string iface   = interfaceSelector.getInterface(selection);
	string localIP = interfaceSelector.getIP(selection);
	if (onInterfaceChanged) onInterfaceChanged(iface, localIP);
}

// -----------------------------------------------------------------------------
// Save / Load
// -----------------------------------------------------------------------------

void GeneralPanel::saveTo(ofJson& config) {
	config["general"]["interface"]     = selectedInterface;
	config["general"]["headless_mode"] = headlessMode;
	config["general"]["collapsed"]     = isCollapsed;
}

void GeneralPanel::loadFrom(ofJson& config) {
	if (!config.contains("general")) return;
	ofJson g = config["general"];

	if (g.contains("interface")) {
		selectInterface(g["interface"].get<string>());
	}
	if (g.contains("headless_mode")) {
		headlessMode = g["headless_mode"].get<bool>();
		if (onHeadlessModeChanged) onHeadlessModeChanged(headlessMode);
	}
	if (g.contains("collapsed")) {
		isCollapsed = g["collapsed"].get<bool>();
	}
}
