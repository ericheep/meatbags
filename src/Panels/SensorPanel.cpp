//
//  SensorPanel.cpp
//  meatbags

#include "SensorPanel.hpp"

const std::vector<std::string>& SensorPanel::sensorTypeNames() {
	static std::vector<std::string> names = { "Hokuyo", "Orbbec Pulsar" };
	return names;
}

const std::vector<int>& SensorPanel::motorSpeedOptions() {
	static std::vector<int> speeds = { 15, 20, 25, 30, 40 };
	return speeds;
}


string SensorPanel::rowLabel(RowType type) {
	switch (type) {
		case RowType::MirrorAngles: return "mirror angles";
		case RowType::ShowInfo:     return "show info";
		case RowType::FogMode:      return "fog mode";
		case RowType::FilterLevel:  return "filter level";
		case RowType::Standby:      return "standby";
		default: return "";
	}
}

SensorPanel::SensorPanel() {
	x = 0; y = 0; width = 220;

	isCollapsed = false;

	scrollY                  = 0.0f;
	bottomMargin             = 20;
	isScrollbarDragging      = false;
	scrollbarDragStartY      = 0.0f;
	scrollbarDragStartScrollY = 0.0f;

	isDragging   = false;
	dragInstance = -1;
	dragRowIndex = -1;

	isEditingFloat    = false;
	editFloatInstance = -1;
	editFloatRow      = -1;
	editFloatText     = "";

	isEditingPos      = false;
	editPosInstance   = -1;
	editPosComponent  = -1;
	editPosText       = "";

	isEditingIP    = false;
	editIPInstance = -1;
	editIPText     = "";

	headerHeight         = 20;
	instanceHeaderHeight = 16;
	rowHeight            = 18;
	checkboxSize         = 10;
	padding              = 6;
	indent               = 8;
	numberBoxW           = 60;  // fixed pixel width for all value boxes
	scrollbarWidth       = 4;

	backgroundColor        = ofColor(0, 0, 0, 220);
	headerColor            = ofColor::thistle;
	headerTextColor        = ofColor::black;
	instanceHeaderColor    = ofColor(40, 40, 40, 255);
	rowColor               = ofColor(20, 20, 20, 240);
	rowAltColor            = ofColor(28, 28, 28, 240);
	controlRowColor        = ofColor(30, 20, 30, 240);  // type, fog, standby, mirror, show info
	dropdownRowColor       = ofColor(30, 20, 30, 240);
	dropdownItemColor      = ofColor(10, 10, 10, 240);
	dropdownItemHoverColor = ofColor(60, 60, 60, 240);
	sliderFillColor        = ofColor(160, 120, 160, 200);
	sliderTrackColor       = ofColor(60, 60, 60, 255);
	numberBoxColor         = ofColor(35, 35, 35, 255);
	numberBoxActiveColor   = ofColor(70, 50, 70, 255);
	ipBoxColor             = ofColor(30, 30, 40, 255);
	ipBoxActiveColor       = ofColor(50, 50, 80, 255);
	textColor              = ofColor::white;
	dimTextColor           = ofColor(160, 160, 160);
	accentColor            = ofColor::thistle;
	connectedColor         = ofColor(80, 200, 80);
	disconnectedColor      = ofColor(200, 80, 80);
	scrollbarColor         = ofColor(30, 30, 30, 180);
	scrollbarThumbColor    = ofColor(120, 100, 120, 220);
}

// -----------------------------------------------------------------------------
// Row helpers
// -----------------------------------------------------------------------------

bool SensorPanel::isOrbbec(Sensor* s) {
	return dynamic_cast<OrbbecPulsarSDK*>(s) != nullptr;
}

std::vector<SensorPanel::RowType> SensorPanel::rowsForSensor(Sensor* s) {
	std::vector<RowType> rows = {
		RowType::TypeDropdown,
		RowType::IPField,
		RowType::WhichMeatbag,
	};
	if (isOrbbec(s)) {
		rows.push_back(RowType::MotorSpeed);
	}
	rows.push_back(RowType::Position);
	rows.push_back(RowType::Rotation);
	if (isOrbbec(s)) {
		rows.push_back(RowType::FilterLevel);
	}
	rows.push_back(RowType::MirrorAngles);
	if (isOrbbec(s)) {
		rows.push_back(RowType::FogMode);
	}
	rows.push_back(RowType::ShowInfo);
	return rows;
}

int SensorPanel::numRows(Sensor* s) {
	return rowsForSensor(s).size();
}

// -----------------------------------------------------------------------------
// Layout
// -----------------------------------------------------------------------------

int SensorPanel::instanceHeight(int i, Sensor* s) {
	if (i < instanceCollapsed.size() && instanceCollapsed[i]) return instanceHeaderHeight;
	return instanceHeaderHeight + numRows(s) * rowHeight;
}

int SensorPanel::instanceYOffset(int i, const std::vector<Sensor*>& sensors) {
	int offset = y + headerHeight - (int)scrollY;
	for (int j = 0; j < i; j++) {
		if (isCollapsed) {
			offset += instanceHeaderHeight;
		} else {
			offset += instanceHeight(j, sensors[j]);
			bool jCollapsed  = j < instanceCollapsed.size()        && instanceCollapsed[j];
			bool jDropOpen   = j < typeDropdownOpen.size()         && typeDropdownOpen[j];
			bool jMsDropOpen = j < motorSpeedDropdownOpen.size()   && motorSpeedDropdownOpen[j];
			bool jWmbDropOpen= j < whichMeatbagDropdownOpen.size() && whichMeatbagDropdownOpen[j];
			if (!jCollapsed) {
				if (jDropOpen)    offset += sensorTypeNames().size()  * rowHeight;
				if (jMsDropOpen)  offset += motorSpeedOptions().size() * rowHeight;
				if (jWmbDropOpen) offset += (sensors[j]->whichMeatbag.getMax() - sensors[j]->whichMeatbag.getMin() + 1) * rowHeight;
			}
		}
	}
	return offset;
}

int SensorPanel::totalContentHeight(const std::vector<Sensor*>& sensors) {
	int h = 0;
	for (int i = 0; i < sensors.size(); i++) {
		// when panel is collapsed, only count instance header heights
		if (isCollapsed) {
			h += instanceHeaderHeight;
		} else {
			h += instanceHeight(i, sensors[i]);
			bool collapsed    = i < instanceCollapsed.size()        && instanceCollapsed[i];
			bool dropOpen     = i < typeDropdownOpen.size()         && typeDropdownOpen[i];
			bool msDropOpen   = i < motorSpeedDropdownOpen.size()   && motorSpeedDropdownOpen[i];
			bool wmbDropOpen  = i < whichMeatbagDropdownOpen.size() && whichMeatbagDropdownOpen[i];
			if (!collapsed) {
				if (dropOpen)    h += sensorTypeNames().size() * rowHeight;
				if (msDropOpen)  h += motorSpeedOptions().size() * rowHeight;
				if (wmbDropOpen) h += (sensors[i]->whichMeatbag.getMax() - sensors[i]->whichMeatbag.getMin() + 1) * rowHeight;
			}
		}
	}
	return h;
}

ofRectangle SensorPanel::panelHeaderRect() {
	return ofRectangle(x, y, width, headerHeight);
}

int SensorPanel::availableHeight() {
	return ofGetHeight() - y - bottomMargin;
}

bool SensorPanel::isScrollable(const std::vector<Sensor*>& sensors) {
	return totalContentHeight(sensors) > availableHeight() - headerHeight;
}

void SensorPanel::clampScroll(const std::vector<Sensor*>& sensors) {
	float maxScroll = (float)(totalContentHeight(sensors) - (availableHeight() - headerHeight));
	scrollY = ofClamp(scrollY, 0.0f, std::max(0.0f, maxScroll));
}

ofRectangle SensorPanel::scrollbarTrackRect() {
	return ofRectangle(x + width - scrollbarWidth, y + headerHeight, scrollbarWidth, availableHeight() - headerHeight);
}

ofRectangle SensorPanel::scrollbarThumbRect(const std::vector<Sensor*>& sensors) {
	ofRectangle track = scrollbarTrackRect();
	int content = totalContentHeight(sensors);
	int avail   = availableHeight() - headerHeight;
	if (content <= 0) return track;
	float thumbRatio = ofClamp((float)avail / content, 0.05f, 1.0f);
	float thumbH     = track.height * thumbRatio;
	float maxScroll  = (float)(content - avail);
	float thumbY     = (maxScroll > 0) ? track.y + (scrollY / maxScroll) * (track.height - thumbH) : track.y;
	return ofRectangle(track.x, thumbY, track.width, thumbH);
}

// -----------------------------------------------------------------------------
// Param helpers
// -----------------------------------------------------------------------------

SensorPanel::SliderInfo SensorPanel::getSliderInfo(RowType type, Sensor* s) {
	SliderInfo info;
	info.isInt = false;
	switch (type) {
		case RowType::Rotation:
			info.label = "rotation";
			info.value = s->sensorRotationDeg;
			info.min   = s->sensorRotationDeg.getMin();
			info.max   = s->sensorRotationDeg.getMax();
			break;
		case RowType::FilterLevel: {
			auto* sdk = dynamic_cast<OrbbecPulsarSDK*>(s);
			if (sdk) {
				info.label = "filter";
				info.value = (float)sdk->guiFilterLevel.get();
				info.min   = (float)sdk->guiFilterLevel.getMin();
				info.max   = (float)sdk->guiFilterLevel.getMax();
				info.isInt = true;
			} else {
				// not an SDK sensor — return rotation as fallback so slider isn't garbage
				info.label = "filter";
				info.value = 0; info.min = 0; info.max = 5; info.isInt = true;
			}
			break;
		}
		default: break;
	}
	return info;
}

void SensorPanel::setSliderValue(RowType type, float value, Sensor* s) {
	switch (type) {
		case RowType::Rotation:      s->sensorRotationDeg = ofClamp(value, s->sensorRotationDeg.getMin(), s->sensorRotationDeg.getMax()); break;
		case RowType::FilterLevel: {
			auto* sdk = dynamic_cast<OrbbecPulsarSDK*>(s);
			if (sdk) sdk->guiFilterLevel = (int)ofClamp(value, sdk->guiFilterLevel.getMin(), sdk->guiFilterLevel.getMax());
			break;
		}
		default: break;
	}
}

bool SensorPanel::getCheckboxValue(RowType type, Sensor* s) {
	switch (type) {
		case RowType::MirrorAngles: return s->mirrorAngles;
		case RowType::ShowInfo:     return s->showSensorInformation;
		case RowType::FogMode:      return s->guiSpecialWorkingMode;
		case RowType::Standby:      return s->isSleeping;
		default: return false;
	}
}

void SensorPanel::setCheckboxValue(RowType type, bool value, Sensor* s) {
	switch (type) {
		case RowType::MirrorAngles: s->mirrorAngles          = value; break;
		case RowType::ShowInfo:     s->showSensorInformation = value; break;
		case RowType::FogMode:      s->guiSpecialWorkingMode = value; break;
		case RowType::Standby:      s->isSleeping            = value; break;
		default: break;
	}
}

// -----------------------------------------------------------------------------
// Draw
// -----------------------------------------------------------------------------

void SensorPanel::closeDropdowns() {
	std::fill(whichMeatbagDropdownOpen.begin(), whichMeatbagDropdownOpen.end(), false);
	std::fill(motorSpeedDropdownOpen.begin(),   motorSpeedDropdownOpen.end(),   false);
	std::fill(typeDropdownOpen.begin(),          typeDropdownOpen.end(),          false);
}

void SensorPanel::draw(const std::vector<Sensor*>& sensors) {
	int n = sensors.size();
	while (typeDropdownOpen.size() < n)        typeDropdownOpen.push_back(false);
	while (typeHoveredItem.size() < n)         typeHoveredItem.push_back("");
	while (motorSpeedDropdownOpen.size() < n)    motorSpeedDropdownOpen.push_back(false);
	while (motorSpeedHoveredItem.size() < n)     motorSpeedHoveredItem.push_back("");
	while (whichMeatbagDropdownOpen.size() < n)  whichMeatbagDropdownOpen.push_back(false);
	while (whichMeatbagHoveredItem.size() < n)   whichMeatbagHoveredItem.push_back("");
	while (instanceCollapsed.size() < n) instanceCollapsed.push_back(false);

	// background — instance headers always visible even when collapsed
	int bgHeight = (n == 0)
				 ? headerHeight
				 : std::min(totalContentHeight(sensors) + headerHeight, availableHeight());
	ofFill();
	ofSetColor(backgroundColor);
	ofDrawRectangle(x, y, width, bgHeight);

	drawHeader();
	if (n == 0) return;

	// scissor to clip content to panel area below header
	int contentAreaY = y + headerHeight;
	int contentAreaH = std::min(totalContentHeight(sensors), availableHeight() - headerHeight);
	float s = (float)ofGetCurrentRenderer()->getViewportWidth() / (float)ofGetWidth();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int)(x * s),
			  (int)((ofGetHeight() - contentAreaY - contentAreaH) * s),
			  (int)(width * s),
			  (int)(contentAreaH * s));

	for (int i = 0; i < n; i++) {
		drawInstance(i, sensors[i], sensors);
	}

	glDisable(GL_SCISSOR_TEST);

	if (!isCollapsed && isScrollable(sensors)) drawScrollbar(sensors);
}

void SensorPanel::drawHeader() {
	ofFill();
	ofSetColor(headerColor);
	ofDrawRectangle(panelHeaderRect());
	ofSetColor(headerTextColor);
	ofDrawBitmapString("sensors", x + padding, y + 14);
	string indicator = isCollapsed ? "+" : "-";
	float indW = indicator.length() * 8;
	ofDrawBitmapString(indicator, x + width - indW - padding, y + 14);
}

void SensorPanel::drawScrollbar(const std::vector<Sensor*>& sensors) {
	// track
	ofRectangle track = scrollbarTrackRect();
	ofFill();
	ofSetColor(scrollbarColor);
	ofDrawRectangle(track);

	// thumb
	ofRectangle thumb = scrollbarThumbRect(sensors);
	ofSetColor(scrollbarThumbColor);
	ofDrawRectangle(thumb);
}

void SensorPanel::drawInstance(int i, Sensor* s, const std::vector<Sensor*>& sensors) {
	int baseY    = instanceYOffset(i, sensors);
	bool collapsed = (i < instanceCollapsed.size()) && instanceCollapsed[i];

	// instance header — sensor color background, black text
	ofRectangle hdrRect(x, baseY, width, instanceHeaderHeight);
	ofFill();
	ofSetColor(s->sensorColor);
	ofDrawRectangle(hdrRect);

	float dotSize = 7;
	float dotX = x + padding;
	float dotY = baseY + (instanceHeaderHeight - dotSize) * 0.5f;
	ofSetColor(s->isConnected ? connectedColor : disconnectedColor);
	ofFill();
	ofDrawRectangle(dotX, dotY, dotSize, dotSize);

	ofSetColor(ofColor::black);
	string label = "sensor " + to_string(s->index);
	if (!s->model.empty()) label += "  " + s->model;
	ofDrawBitmapString(label, x + padding + dotSize + 4, baseY + 11);
	// +/- right-aligned
	string indicator = collapsed ? "+" : "-";
	float indW = indicator.length() * 8;
	ofDrawBitmapString(indicator, x + width - indW - padding, baseY + 11);

	if (collapsed) return;  // skip content rows but header already drawn above

	int contentY  = baseY + instanceHeaderHeight;
	bool dropOpen = typeDropdownOpen[i];
	auto rows     = rowsForSensor(s);

	float rowRight = (float)(x + width);
	float rowWidth = (float)(width - indent);

	// type dropdown row
	ofRectangle typeRow(x + indent, contentY, rowWidth, rowHeight);
	ofFill();
	ofSetColor(controlRowColor);
	ofDrawRectangle(typeRow);
	ofSetColor(ofColor(80, 80, 80));
	ofDrawLine(typeRow.x, typeRow.getBottom(), typeRow.getRight(), typeRow.getBottom());

	string currentType = dynamic_cast<OrbbecPulsarSDK*>(s) ? "Orbbec Pulsar"
					   : false ? "Orbbec Pulsar"
					   : "Hokuyo";
	ofSetColor(textColor);
	ofDrawBitmapString("type", typeRow.x + padding, typeRow.y + 13);
	float typeValW = currentType.length() * 8;
	ofSetColor(dimTextColor);
	ofDrawBitmapString(currentType, rowRight - typeValW - padding, typeRow.y + 13);

	contentY += rowHeight;

	// type dropdown items
	if (dropOpen) {
		for (int j = 0; j < sensorTypeNames().size(); j++) {
			string name = sensorTypeNames()[j];
			ofRectangle itemRect(x + indent, contentY, rowWidth, rowHeight);
			bool hovered = (typeHoveredItem[i] == name);

			ofFill();
			ofSetColor(hovered ? dropdownItemHoverColor : dropdownItemColor);
			ofDrawRectangle(itemRect);

			if (name == currentType) {
				ofSetColor(accentColor);
				ofDrawBitmapString("*", itemRect.x + padding, itemRect.y + 13);
			}
			ofSetColor(hovered ? textColor : dimTextColor);
			ofDrawBitmapString(name, itemRect.x + padding + 12, itemRect.y + 13);
			ofSetColor(ofColor(50, 50, 50));
			ofDrawLine(itemRect.x, itemRect.getBottom(), itemRect.getRight(), itemRect.getBottom());
			contentY += rowHeight;
		}
	}

	// remaining rows
	for (int r = 1; r < rows.size(); r++) {
		RowType type = rows[r];
		ofRectangle row(x + indent, contentY, rowWidth, rowHeight);

		if (type == RowType::IPField) {
			bool editing   = isEditingIP && editIPInstance == i;
			string display = editing ? editIPText + "|" : s->ipAddress.get();
			if (display.empty() && !editing) display = "0.0.0.0";

			ofFill();
			ofSetColor(editing ? ipBoxActiveColor : rowColor);
			ofDrawRectangle(row);
			ofNoFill();
			ofSetColor(editing ? accentColor : ofColor(70, 70, 70));
			ofDrawRectangle(row);
			ofFill();

			ofSetColor(dimTextColor);
			ofDrawBitmapString("IP", row.x + padding, row.y + 13);
			float ipW = display.length() * 8;
			ofSetColor(editing ? textColor : dimTextColor);
			ofDrawBitmapString(display, rowRight - ipW - padding, row.y + 13);

		} else if (type == RowType::Position) {
			// "pos"  [x box]  [y box] — same width as all other number boxes
			float labelW = rowWidth - 2.0f * numberBoxW;
			ofRectangle xBox(row.x + labelW,              row.y, numberBoxW, row.height);
			ofRectangle yBox(row.x + labelW + numberBoxW, row.y, numberBoxW, row.height);

			bool editX = isEditingPos && editPosInstance == i && editPosComponent == 0;
			bool editY = isEditingPos && editPosInstance == i && editPosComponent == 1;

			ofFill();
			ofSetColor(r % 2 == 0 ? rowColor : rowAltColor);
			ofDrawRectangle(row);

			ofSetColor(textColor);
			ofDrawBitmapString("pos", row.x + padding, row.y + 13);

			// x box
			ofFill();
			ofSetColor(editX ? numberBoxActiveColor : numberBoxColor);
			ofDrawRectangle(xBox);
			ofNoFill();
			ofSetColor(editX ? accentColor : ofColor(70, 70, 70));
			ofDrawRectangle(xBox);
			ofFill();
			string xStr = editX ? editPosText + "|" : ofToString(s->positionX.get(), 2);
			ofSetColor(editX ? textColor : dimTextColor);
			ofDrawBitmapString(xStr, xBox.getRight() - xStr.length() * 8 - padding, row.y + 13);

			// y box
			ofFill();
			ofSetColor(editY ? numberBoxActiveColor : numberBoxColor);
			ofDrawRectangle(yBox);
			ofNoFill();
			ofSetColor(editY ? accentColor : ofColor(70, 70, 70));
			ofDrawRectangle(yBox);
			ofFill();
			string yStr = editY ? editPosText + "|" : ofToString(s->positionY.get(), 2);
			ofSetColor(editY ? textColor : dimTextColor);
			ofDrawBitmapString(yStr, yBox.getRight() - yStr.length() * 8 - padding, row.y + 13);

		} else if (type == RowType::MirrorAngles || type == RowType::ShowInfo ||
				   type == RowType::FogMode       || type == RowType::Standby) {
			ofFill();
			ofSetColor(controlRowColor);
			ofDrawRectangle(row);

			bool val = getCheckboxValue(type, s);
			ofRectangle checkRect(row.x + padding, row.y + (rowHeight - checkboxSize) * 0.5f, checkboxSize, checkboxSize);
			ofNoFill();
			ofSetColor(accentColor);
			ofDrawRectangle(checkRect);

			if (val) {
				ofFill();
				ofSetColor(accentColor);
				ofRectangle inner = checkRect;
				inner.scaleFromCenter(0.6f);
				ofDrawRectangle(inner);
			}

			ofFill();
			ofSetColor(textColor);
			ofDrawBitmapString(rowLabel(type), checkRect.getRight() + padding, row.y + 13);

		} else if (type == RowType::WhichMeatbag) {
			// meatbag dropdown — options populated from whichMeatbag parameter range
			bool wmbDropOpen = whichMeatbagDropdownOpen[i];
			int  current     = s->whichMeatbag.get();
			int  maxMeatbag  = s->whichMeatbag.getMax();

			ofFill();
			ofSetColor(controlRowColor);
			ofDrawRectangle(row);
			ofSetColor(ofColor(80, 80, 80));
			ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());

			ofSetColor(textColor);
			ofDrawBitmapString("meatbag", row.x + padding, row.y + 13);
			string valStr = ofToString(current);
			float  valW   = valStr.length() * 8;
			ofSetColor(dimTextColor);
			ofDrawBitmapString(valStr, rowRight - valW - padding, row.y + 13);

			contentY += rowHeight;

			if (wmbDropOpen) {
				for (int m = s->whichMeatbag.getMin(); m <= maxMeatbag; m++) {
					ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
					bool hovered = (whichMeatbagHoveredItem[i] == ofToString(m));
					ofFill();
					ofSetColor(hovered ? dropdownItemHoverColor : dropdownItemColor);
					ofDrawRectangle(itemRect);
					if (m == current) {
						ofSetColor(accentColor);
						ofDrawBitmapString("*", itemRect.x + padding, itemRect.y + 13);
					}
					ofSetColor(hovered ? textColor : dimTextColor);
					ofDrawBitmapString(ofToString(m), itemRect.x + padding + 12, itemRect.y + 13);
					ofSetColor(ofColor(50, 50, 50));
					ofDrawLine(itemRect.x, itemRect.getBottom(), itemRect.getRight(), itemRect.getBottom());
					contentY += rowHeight;
				}
			}
			continue;

		} else if (type == RowType::MotorSpeed) {
			// motor speed dropdown — fixed options: 15, 20, 25, 30, 40 Hz
			bool msDropOpen = motorSpeedDropdownOpen[i];
			int  currentHz  = s->guiMotorSpeed.get();
			if (currentHz == 0) currentHz = 20;  // default

			ofFill();
			ofSetColor(controlRowColor);
			ofDrawRectangle(row);
			ofSetColor(ofColor(80, 80, 80));
			ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());

			ofSetColor(textColor);
			ofDrawBitmapString("speed", row.x + padding, row.y + 13);
			string hzStr = ofToString(currentHz) + " Hz";
			float  hzW   = hzStr.length() * 8;
			ofSetColor(dimTextColor);
			ofDrawBitmapString(hzStr, rowRight - hzW - padding, row.y + 13);

			contentY += rowHeight;

			if (msDropOpen) {
				for (int hz : motorSpeedOptions()) {
					ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
					bool hovered = (motorSpeedHoveredItem[i] == ofToString(hz));
					ofFill();
					ofSetColor(hovered ? dropdownItemHoverColor : dropdownItemColor);
					ofDrawRectangle(itemRect);
					if (hz == currentHz) {
						ofSetColor(accentColor);
						ofDrawBitmapString("*", itemRect.x + padding, itemRect.y + 13);
					}
					ofSetColor(hovered ? textColor : dimTextColor);
					ofDrawBitmapString(ofToString(hz) + " Hz", itemRect.x + padding + 12, itemRect.y + 13);
					ofSetColor(ofColor(50, 50, 50));
					ofDrawLine(itemRect.x, itemRect.getBottom(), itemRect.getRight(), itemRect.getBottom());
					contentY += rowHeight;
				}
			}

			ofSetColor(ofColor(50, 50, 50));
			// don't increment contentY here — already done above
			continue;

		} else if (type == RowType::FilterLevel) {
			auto* sdk = dynamic_cast<OrbbecPulsarSDK*>(s);
			int level = sdk ? sdk->guiFilterLevel.get() : 0;
			int minL  = sdk ? sdk->guiFilterLevel.getMin() : 0;
			int maxL  = sdk ? sdk->guiFilterLevel.getMax() : 5;

			ofFill();
			ofSetColor(controlRowColor);
			ofDrawRectangle(row);

			ofSetColor(textColor);
			ofDrawBitmapString("filter level", row.x + padding, row.y + 13);

			float btnW    = 16.0f;
			float btnRight = rowRight - padding;
			ofRectangle plusRect (btnRight - btnW,              row.y + 1, btnW,      rowHeight - 2);
			ofRectangle valRect  (plusRect.x  - btnW - 2,       row.y + 1, btnW,      rowHeight - 2);
			ofRectangle minusRect(valRect.x   - btnW - 2,       row.y + 1, btnW,      rowHeight - 2);

			ofFill();
			ofSetColor(level > minL ? numberBoxColor : ofColor(25, 25, 25));
			ofDrawRectangle(minusRect);
			ofSetColor(level < maxL ? numberBoxColor : ofColor(25, 25, 25));
			ofDrawRectangle(plusRect);

			ofSetColor(dimTextColor);
			ofDrawBitmapString("-", minusRect.x + 4, minusRect.y + 13);
			ofDrawBitmapString("+", plusRect.x  + 3, plusRect.y  + 13);
			ofSetColor(textColor);
			ofDrawBitmapString(ofToString(level), valRect.x + (btnW * 0.5f) - 4, valRect.y + 13);

			ofSetColor(ofColor(50, 50, 50));
			ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());
			contentY += rowHeight;
			continue;

		} else {
			// slider + number box
			bool dragging = isDragging && dragInstance == i && dragRowIndex == r;
			bool editing  = isEditingFloat && editFloatInstance == i && editFloatRow == r;
			SliderInfo info = getSliderInfo(type, s);

			ofRectangle slRect(row.x,               row.y, rowWidth - numberBoxW, row.height);
			ofRectangle nbRect(rowRight - numberBoxW, row.y, numberBoxW,           row.height);

			ofFill();
			ofSetColor(r % 2 == 0 ? rowColor : rowAltColor);
			ofDrawRectangle(row);

			// slider
			float t = (info.max > info.min) ? (info.value - info.min) / (info.max - info.min) : 0.0f;
			ofSetColor(sliderTrackColor);
			ofDrawRectangle(slRect);
			ofSetColor(dragging ? ofColor(180, 140, 180, 210) : sliderFillColor);
			ofDrawRectangle(slRect.x, slRect.y, slRect.width * t, slRect.height);
			ofSetColor(textColor);
			ofDrawBitmapString(info.label, slRect.x + padding, slRect.y + 13);

			// number box
			ofFill();
			ofSetColor(editing ? numberBoxActiveColor : numberBoxColor);
			ofDrawRectangle(nbRect);
			ofNoFill();
			ofSetColor(editing ? accentColor : ofColor(70, 70, 70));
			ofDrawRectangle(nbRect);
			ofFill();

			string display = editing ? editFloatText + "|"
						   : (info.isInt ? ofToString((int)info.value) : ofToString(info.value, 2));
			ofSetColor(editing ? textColor : dimTextColor);
			ofDrawBitmapString(display, rowRight - display.length() * 8 - padding, row.y + 13);
		}

		ofSetColor(ofColor(50, 50, 50));
		ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());
		contentY += rowHeight;
	}
}


// -----------------------------------------------------------------------------
// Mouse
// -----------------------------------------------------------------------------

bool SensorPanel::onMousePressed(ofMouseEventArgs& args, const std::vector<Sensor*>& sensors) {
	ofPoint mouse(args.x, args.y);
	int n = sensors.size();
	while (typeDropdownOpen.size() < n)        typeDropdownOpen.push_back(false);
	while (typeHoveredItem.size() < n)         typeHoveredItem.push_back("");
	while (motorSpeedDropdownOpen.size() < n)    motorSpeedDropdownOpen.push_back(false);
	while (motorSpeedHoveredItem.size() < n)     motorSpeedHoveredItem.push_back("");
	while (whichMeatbagDropdownOpen.size() < n)  whichMeatbagDropdownOpen.push_back(false);
	while (whichMeatbagHoveredItem.size() < n)   whichMeatbagHoveredItem.push_back("");
	while (instanceCollapsed.size() < n) instanceCollapsed.push_back(false);

	if (panelHeaderRect().inside(mouse)) {
		isCollapsed = !isCollapsed;
		scrollY = 0.0f;
		// collapse/expand all instances together
		for (int j = 0; j < instanceCollapsed.size(); j++) {
			instanceCollapsed[j] = isCollapsed;
		}
		isEditingFloat = isEditingPos = isEditingIP = false;
		return true;
	}

	// scrollbar thumb drag
	if (isScrollable(sensors) && scrollbarThumbRect(sensors).inside(mouse)) {
		isScrollbarDragging       = true;
		scrollbarDragStartY       = mouse.y;
		scrollbarDragStartScrollY = scrollY;
		return true;
	}

	for (int i = 0; i < n; i++) {
		int baseY = instanceYOffset(i, sensors);

		// click connection dot to force reconnect
		float dotSize = 7;
		float dotX    = x + padding + dotSize * 0.5f;
		float dotY    = baseY + instanceHeaderHeight * 0.5f;
		if (ofPoint(mouse.x, mouse.y).distance(ofPoint(dotX, dotY)) <= dotSize) {
			sensors[i]->reconnect();
			return true;
		}

		ofRectangle hdrRect(x, baseY, width, instanceHeaderHeight);
		if (hdrRect.inside(mouse)) {
			instanceCollapsed[i] = !instanceCollapsed[i];
			typeDropdownOpen[i]  = false;
			// expanding an instance while panel is collapsed implicitly expands the panel
			if (!instanceCollapsed[i] && isCollapsed) isCollapsed = false;
			isEditingFloat = isEditingPos = isEditingIP = false;
			return true;
		}

		if (instanceCollapsed[i]) continue;

		int contentY  = baseY + instanceHeaderHeight;
		auto rows     = rowsForSensor(sensors[i]);
		bool dropOpen = typeDropdownOpen[i];

		// type dropdown row
		ofRectangle typeRow(x + indent, contentY, width - indent, rowHeight);
		if (typeRow.inside(mouse)) {
			typeDropdownOpen[i] = !typeDropdownOpen[i];
			isEditingFloat = isEditingPos = isEditingIP = false;
			return true;
		}
		contentY += rowHeight;

		// type dropdown items
		if (dropOpen) {
			for (int j = 0; j < sensorTypeNames().size(); j++) {
				ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
				if (itemRect.inside(mouse)) {
					typeDropdownOpen[i] = false;
					if (onSensorTypeChanged) onSensorTypeChanged(i, sensorTypeNames()[j]);
					return true;
				}
				contentY += rowHeight;
			}
			typeDropdownOpen[i] = false;
			return true;
		}

		float rowRight = x + width;

		for (int r = 1; r < rows.size(); r++) {
			RowType type = rows[r];
			ofRectangle row(x + indent, contentY, width - indent, rowHeight);

			if (type == RowType::IPField) {
				if (row.inside(mouse)) {
					isEditingIP    = true;
					editIPInstance = i;
					editIPText     = sensors[i]->ipAddress.get();
					isEditingFloat = isEditingPos = false;
					return true;
				}

			} else if (type == RowType::Position) {
				float rowW   = width - indent;
				float labelW = rowW - 2.0f * numberBoxW;
				ofRectangle xBox(row.x + labelW,              row.y, numberBoxW, row.height);
				ofRectangle yBox(row.x + labelW + numberBoxW, row.y, numberBoxW, row.height);

				if (xBox.inside(mouse)) {
					isEditingPos      = true;
					editPosInstance   = i;
					editPosComponent  = 0;
					editPosText       = ofToString(sensors[i]->positionX.get(), 2);
					isEditingFloat = isEditingIP = isDragging = false;
					return true;
				}
				if (yBox.inside(mouse)) {
					isEditingPos      = true;
					editPosInstance   = i;
					editPosComponent  = 1;
					editPosText       = ofToString(sensors[i]->positionY.get(), 2);
					isEditingFloat = isEditingIP = isDragging = false;
					return true;
				}

			} else if (type == RowType::WhichMeatbag) {
				ofRectangle row(x + indent, contentY, width - indent, rowHeight);
				if (row.inside(mouse)) {
					whichMeatbagDropdownOpen[i] = !whichMeatbagDropdownOpen[i];
					return true;
				}
				contentY += rowHeight;
				if (whichMeatbagDropdownOpen[i]) {
					for (int m = sensors[i]->whichMeatbag.getMin(); m <= sensors[i]->whichMeatbag.getMax(); m++) {
						ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
						if (itemRect.inside(mouse)) {
							sensors[i]->whichMeatbag = m;
							whichMeatbagDropdownOpen[i] = false;
							return true;
						}
						contentY += rowHeight;
					}
					whichMeatbagDropdownOpen[i] = false;
					return true;
				}
				continue;

			} else if (type == RowType::MotorSpeed) {
				ofRectangle row(x + indent, contentY, width - indent, rowHeight);
				if (row.inside(mouse)) {
					motorSpeedDropdownOpen[i] = !motorSpeedDropdownOpen[i];
					return true;
				}
				contentY += rowHeight;
				if (motorSpeedDropdownOpen[i]) {
					for (int hz : motorSpeedOptions()) {
						ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
						if (itemRect.inside(mouse)) {
							sensors[i]->guiMotorSpeed = hz;
							motorSpeedDropdownOpen[i] = false;
							return true;
						}
						contentY += rowHeight;
					}
					motorSpeedDropdownOpen[i] = false;
					return true;
				}
				continue;

			} else if (type == RowType::FilterLevel) {
				auto* sdk = dynamic_cast<OrbbecPulsarSDK*>(sensors[i]);
				if (sdk) {
					float btnW    = 16.0f;
					float btnRight = (float)(x + width) - padding;
					ofRectangle plusRect (btnRight - btnW,        row.y + 1, btnW, rowHeight - 2);
					ofRectangle valRect  (plusRect.x - btnW - 2,  row.y + 1, btnW, rowHeight - 2);
					ofRectangle minusRect(valRect.x  - btnW - 2,  row.y + 1, btnW, rowHeight - 2);
					if (plusRect.inside(mouse)) {
						sdk->guiFilterLevel = ofClamp(sdk->guiFilterLevel.get() + 1,
													  sdk->guiFilterLevel.getMin(),
													  sdk->guiFilterLevel.getMax());
						return true;
					}
					if (minusRect.inside(mouse)) {
						sdk->guiFilterLevel = ofClamp(sdk->guiFilterLevel.get() - 1,
													  sdk->guiFilterLevel.getMin(),
													  sdk->guiFilterLevel.getMax());
						return true;
					}
				}
				contentY += rowHeight;
				continue;

			} else if (type == RowType::MirrorAngles || type == RowType::ShowInfo ||
					   type == RowType::FogMode       || type == RowType::Standby) {
				if (row.inside(mouse)) {
					setCheckboxValue(type, !getCheckboxValue(type, sensors[i]), sensors[i]);
					return true;
				}

			} else {
				float boxW  = numberBoxW;
				ofRectangle slRect(row.x, row.y, row.width - boxW, row.height);
				ofRectangle nbRect(rowRight - boxW, row.y, boxW, row.height);

				if (nbRect.inside(mouse)) {
					isEditingFloat    = true;
					editFloatInstance = i;
					editFloatRow      = r;
					SliderInfo info   = getSliderInfo(type, sensors[i]);
					editFloatText     = info.isInt ? ofToString((int)info.value) : ofToString(info.value, 2);
					isEditingPos = isEditingIP = isDragging = false;
					return true;
				}
				if (slRect.inside(mouse)) {
					isEditingFloat = isEditingPos = isEditingIP = false;
					isDragging   = true;
					dragInstance = i;
					dragRowIndex = r;
					SliderInfo info = getSliderInfo(type, sensors[i]);
					float t = ofClamp((mouse.x - slRect.x) / slRect.width, 0.0f, 1.0f);
					setSliderValue(type, info.min + t * (info.max - info.min), sensors[i]);
					return true;
				}
			}

			contentY += rowHeight;
		}
	}

	// click outside — confirm edits
	if (isEditingFloat) {
		try {
			auto rows = rowsForSensor(sensors[editFloatInstance]);
			setSliderValue(rows[editFloatRow], std::stof(editFloatText), sensors[editFloatInstance]);
		} catch (...) {}
		isEditingFloat = false;
	}
	if (isEditingPos) {
		try {
			float val = std::stof(editPosText);
			if (editPosComponent == 0) sensors[editPosInstance]->positionX = val;
			else                       sensors[editPosInstance]->positionY = val;
		} catch (...) {}
		isEditingPos = false;
	}
	if (isEditingIP) {
		sensors[editIPInstance]->ipAddress = editIPText;
		isEditingIP = false;
	}

	return false;
}

void SensorPanel::onMouseDragged(ofMouseEventArgs& args, const std::vector<Sensor*>& sensors) {
	// scrollbar thumb drag
	if (isScrollbarDragging) {
		ofRectangle track   = scrollbarTrackRect();
		int content         = totalContentHeight(sensors);
		int avail           = availableHeight() - headerHeight;
		float maxScroll     = (float)(content - avail);
		if (maxScroll > 0 && track.height > 0) {
			float thumbH    = track.height * ofClamp((float)avail / content, 0.05f, 1.0f);
			float trackTravel = track.height - thumbH;
			float delta     = args.y - scrollbarDragStartY;
			scrollY = scrollbarDragStartScrollY + (delta / trackTravel) * maxScroll;
			clampScroll(sensors);
		}
		return;
	}

	if (!isDragging) return;
	if (dragInstance < 0 || dragInstance >= sensors.size()) return;

	Sensor* s    = sensors[dragInstance];
	auto rows    = rowsForSensor(s);
	RowType type = rows[dragRowIndex];

	int baseY    = instanceYOffset(dragInstance, sensors);
	int contentY = baseY + instanceHeaderHeight + rowHeight;
	bool dropOpen = dragInstance < typeDropdownOpen.size() && typeDropdownOpen[dragInstance];
	if (dropOpen) contentY += sensorTypeNames().size() * rowHeight;
	for (int r = 1; r < dragRowIndex; r++) contentY += rowHeight;

	float boxW  = numberBoxW;
	ofRectangle slRect(x + indent, contentY, (width - indent) - boxW, rowHeight);

	SliderInfo info = getSliderInfo(type, s);
	float t = ofClamp((args.x - slRect.x) / slRect.width, 0.0f, 1.0f);
	setSliderValue(type, info.min + t * (info.max - info.min), s);
}

bool SensorPanel::onMouseScrolled(ofMouseEventArgs& args, const std::vector<Sensor*>& sensors) {
	if (isCollapsed) return false;

	// only scroll if mouse is over the panel
	ofRectangle panelArea(x, y, width, availableHeight());
	if (!panelArea.inside(ofPoint(args.x, args.y))) return false;

	if (!isScrollable(sensors)) return false;

	scrollY -= args.scrollY * rowHeight * 2;
	clampScroll(sensors);
	return true;
}

bool SensorPanel::onMouseReleased(ofMouseEventArgs& args) {
	if (isScrollbarDragging) {
		isScrollbarDragging = false;
		return true;
	}
	if (isDragging) {
		isDragging   = false;
		dragInstance = -1;
		dragRowIndex = -1;
		return true;
	}
	return false;
}

void SensorPanel::onMouseMoved(ofMouseEventArgs& args, const std::vector<Sensor*>& sensors) {
	ofPoint mouse(args.x, args.y);
	int n = sensors.size();
	while (typeDropdownOpen.size() < n)        typeDropdownOpen.push_back(false);
	while (typeHoveredItem.size() < n)         typeHoveredItem.push_back("");
	while (motorSpeedDropdownOpen.size() < n)    motorSpeedDropdownOpen.push_back(false);
	while (motorSpeedHoveredItem.size() < n)     motorSpeedHoveredItem.push_back("");
	while (whichMeatbagDropdownOpen.size() < n)  whichMeatbagDropdownOpen.push_back(false);
	while (whichMeatbagHoveredItem.size() < n)   whichMeatbagHoveredItem.push_back("");
	while (instanceCollapsed.size() < n) instanceCollapsed.push_back(false);

	for (int i = 0; i < n; i++) {
		typeHoveredItem[i]        = "";
		motorSpeedHoveredItem[i]  = "";
		whichMeatbagHoveredItem[i]= "";

		int baseY    = instanceYOffset(i, sensors);
		int contentY = baseY + instanceHeaderHeight + rowHeight;

		if (typeDropdownOpen[i]) {
			for (int j = 0; j < sensorTypeNames().size(); j++) {
				ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
				if (itemRect.inside(mouse)) {
					typeHoveredItem[i] = sensorTypeNames()[j];
					break;
				}
				contentY += rowHeight;
			}
		}

		if (motorSpeedDropdownOpen[i]) {
			// skip to motor speed row: after type row + dropdown items + other rows
			// simplest: rebuild contentY up to motor speed dropdown items
			auto rows     = rowsForSensor(sensors[i]);
			bool tDropOpen = typeDropdownOpen[i];
			int msContentY = baseY + instanceHeaderHeight + rowHeight;
			if (tDropOpen) msContentY += sensorTypeNames().size() * rowHeight;
			for (int r = 1; r < (int)rows.size(); r++) {
				if (rows[r] == RowType::MotorSpeed) {
					msContentY += rowHeight; // skip the dropdown header row
					for (int hz : motorSpeedOptions()) {
						ofRectangle itemRect(x + indent, msContentY, width - indent, rowHeight);
						if (itemRect.inside(mouse)) {
							motorSpeedHoveredItem[i] = ofToString(hz);
							break;
						}
						msContentY += rowHeight;
					}
					break;
				}
				msContentY += rowHeight;
			}
		}
		if (whichMeatbagDropdownOpen[i]) {
			auto rows      = rowsForSensor(sensors[i]);
			bool tDropOpen = typeDropdownOpen[i];
			int wmbContentY = baseY + instanceHeaderHeight + rowHeight;
			if (tDropOpen) wmbContentY += sensorTypeNames().size() * rowHeight;
			for (int r = 1; r < (int)rows.size(); r++) {
				if (rows[r] == RowType::WhichMeatbag) {
					wmbContentY += rowHeight;
					for (int m = sensors[i]->whichMeatbag.getMin(); m <= sensors[i]->whichMeatbag.getMax(); m++) {
						ofRectangle itemRect(x + indent, wmbContentY, width - indent, rowHeight);
						if (itemRect.inside(mouse)) {
							whichMeatbagHoveredItem[i] = ofToString(m);
							break;
						}
						wmbContentY += rowHeight;
					}
					break;
				}
				wmbContentY += rowHeight;
			}
		}
	}
}
// -----------------------------------------------------------------------------

bool SensorPanel::onKeyPressed(ofKeyEventArgs& args, const std::vector<Sensor*>& sensors) {
	int key = args.key;

	if (isEditingFloat) {
		if (key == OF_KEY_RETURN || key == OF_KEY_TAB) {
			try {
				auto rows = rowsForSensor(sensors[editFloatInstance]);
				setSliderValue(rows[editFloatRow], std::stof(editFloatText), sensors[editFloatInstance]);
			} catch (...) {}
			isEditingFloat = false;
			return true;
		}
		if (key == OF_KEY_ESC)       { isEditingFloat = false; return true; }
		if (key == OF_KEY_BACKSPACE) { if (!editFloatText.empty()) editFloatText.pop_back(); return true; }
		char c = (char)key;
		if (std::isdigit(c) || c == '.') { editFloatText += c; return true; }
		return false;
	}

	if (isEditingPos) {
		if (key == OF_KEY_RETURN || key == OF_KEY_TAB) {
			try {
				float val = std::stof(editPosText);
				if (editPosComponent == 0) sensors[editPosInstance]->positionX = val;
				else                       sensors[editPosInstance]->positionY = val;
			} catch (...) {}
			isEditingPos = false;
			return true;
		}
		if (key == OF_KEY_ESC)       { isEditingPos = false; return true; }
		if (key == OF_KEY_BACKSPACE) { if (!editPosText.empty()) editPosText.pop_back(); return true; }
		char c = (char)key;
		if (std::isdigit(c) || c == '-' || c == '.') { editPosText += c; return true; }
		return false;
	}

	if (isEditingIP) {
		if (key == OF_KEY_RETURN || key == OF_KEY_TAB) {
			sensors[editIPInstance]->ipAddress = editIPText;
			isEditingIP = false;
			return true;
		}
		if (key == OF_KEY_ESC)       { isEditingIP = false; return true; }
		if (key == OF_KEY_BACKSPACE) { if (!editIPText.empty()) editIPText.pop_back(); return true; }
		char c = (char)key;
		if (std::isdigit(c) || c == '.' || std::isalpha(c)) { editIPText += c; return true; }
		return false;
	}

	return false;
}

// -----------------------------------------------------------------------------
// Save / Load
// -----------------------------------------------------------------------------

void SensorPanel::saveTo(ofJson& config) {
	config["sensor_panel"]["collapsed"] = isCollapsed;
	config["sensor_panel"]["scroll_y"]  = scrollY;

	ofJson instances = ofJson::array();
	for (int i = 0; i < instanceCollapsed.size(); i++) {
		instances.push_back(instanceCollapsed[i]);
	}
	config["sensor_panel"]["instance_collapsed"] = instances;
}

void SensorPanel::loadFrom(ofJson& config) {
	if (!config.contains("sensor_panel")) return;
	ofJson g = config["sensor_panel"];

	if (g.contains("collapsed")) isCollapsed = g["collapsed"].get<bool>();
	if (g.contains("scroll_y"))  scrollY     = g["scroll_y"].get<float>();

	if (g.contains("instance_collapsed")) {
		instanceCollapsed.clear();
		for (auto& val : g["instance_collapsed"]) {
			instanceCollapsed.push_back(val.get<bool>());
		}
	}
}
