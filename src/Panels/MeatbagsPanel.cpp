//
//  MeatbagsPanel.cpp
//  meatbags

#include "MeatbagsPanel.hpp"

static const int NUM_PARAMS = 4;
static const char* PARAM_LABELS[] = { "epsilon", "min points", "persistence", "vel smooth" };

const std::vector<std::string>& MeatbagsPanel::clustererNames() {
	static std::vector<std::string> names = { "DBSCAN", "Euclidean" };
	return names;
}

MeatbagsPanel::MeatbagsPanel() {
	x = 0;
	y = 0;
	width = 220;

	isCollapsed = false;

	isDragging   = false;
	dragInstance = -1;
	dragParam    = -1;

	isEditing    = false;
	editInstance = -1;
	editParam    = -1;
	editText     = "";

	headerHeight         = 20;
	instanceHeaderHeight = 16;
	rowHeight            = 18;
	padding              = 6;
	indent               = 8;
	numberBoxRatio       = 0.283f;

	backgroundColor       = ofColor(0, 0, 0, 220);
	headerColor           = ofColor::thistle;
	headerTextColor       = ofColor::black;
	instanceHeaderColor   = ofColor(40, 40, 40, 255);
	rowColor              = ofColor(20, 20, 20, 240);
	rowAltColor           = ofColor(28, 28, 28, 240);
	dropdownRowColor      = ofColor(30, 20, 30, 240);
	dropdownItemColor     = ofColor(10, 10, 10, 240);
	dropdownItemHoverColor = ofColor(60, 60, 60, 240);
	sliderFillColor       = ofColor(160, 120, 160, 200);
	sliderTrackColor      = ofColor(60, 60, 60, 255);
	numberBoxColor        = ofColor(35, 35, 35, 255);
	numberBoxActiveColor  = ofColor(70, 50, 70, 255);
	textColor             = ofColor::white;
	dimTextColor          = ofColor(160, 160, 160);
	accentColor           = ofColor::thistle;
}

// -----------------------------------------------------------------------------
// Layout
// -----------------------------------------------------------------------------

int MeatbagsPanel::instanceHeight() {
	// header + clusterer row + NUM_PARAMS rows
	return instanceHeaderHeight + rowHeight + NUM_PARAMS * rowHeight;
}

int MeatbagsPanel::instanceYOffset(int i) {
	// account for any open dropdowns above this instance
	int offset = headerHeight;
	for (int j = 0; j < i; j++) {
		offset += instanceHeight();
		if (j < clustererDropdownOpen.size() && clustererDropdownOpen[j]) {
			offset += clustererNames().size() * rowHeight;
		}
	}
	return offset;
}

int MeatbagsPanel::totalHeight(int n) {
	if (isCollapsed) return headerHeight;
	int h = headerHeight;
	for (int i = 0; i < n; i++) {
		h += instanceHeight();
		if (i < clustererDropdownOpen.size() && clustererDropdownOpen[i]) {
			h += clustererNames().size() * rowHeight;
		}
	}
	return h;
}

ofRectangle MeatbagsPanel::panelHeaderRect() {
	return ofRectangle(x, y, width, headerHeight);
}

ofRectangle MeatbagsPanel::instanceHeaderRect(int i) {
	return ofRectangle(x, y + instanceYOffset(i), width, instanceHeaderHeight);
}

ofRectangle MeatbagsPanel::clustererRowRect(int i) {
	return ofRectangle(x + indent, y + instanceYOffset(i) + instanceHeaderHeight, width - indent, rowHeight);
}

ofRectangle MeatbagsPanel::clustererItemRect(int i, int itemIndex) {
	int itemY = y + instanceYOffset(i) + instanceHeaderHeight + rowHeight + itemIndex * rowHeight;
	return ofRectangle(x + indent, itemY, width - indent, rowHeight);
}

ofRectangle MeatbagsPanel::paramRowRect(int i, int p) {
	int extraY = 0;
	if (i < clustererDropdownOpen.size() && clustererDropdownOpen[i]) {
		extraY = clustererNames().size() * rowHeight;
	}
	int rowY = y + instanceYOffset(i) + instanceHeaderHeight + rowHeight + extraY + p * rowHeight;
	return ofRectangle(x + indent, rowY, width - indent, rowHeight);
}

ofRectangle MeatbagsPanel::sliderRect(int i, int p) {
	ofRectangle row = paramRowRect(i, p);
	float boxW = row.width * numberBoxRatio;
	return ofRectangle(row.x, row.y, row.width - boxW, row.height);
}

ofRectangle MeatbagsPanel::numberBoxRect(int i, int p) {
	ofRectangle row = paramRowRect(i, p);
	float boxW = row.width * numberBoxRatio;
	return ofRectangle(row.getRight() - boxW, row.y, boxW, row.height);
}

// -----------------------------------------------------------------------------
// Param helpers
// -----------------------------------------------------------------------------

MeatbagsPanel::ParamInfo MeatbagsPanel::getParamInfo(int p, Meatbags* m) {
	ParamInfo info;
	info.label = PARAM_LABELS[p];
	switch (p) {
		case 0: info.value = m->epsilon;         info.min = m->epsilon.getMin();         info.max = m->epsilon.getMax();         break;
		case 1: info.value = m->minPoints;        info.min = m->minPoints.getMin();       info.max = m->minPoints.getMax();       break;
		case 2: info.value = m->blobPersistence;   info.min = m->blobPersistence.getMin();   info.max = m->blobPersistence.getMax();   break;
		case 3: info.value = m->velocitySmoothing; info.min = m->velocitySmoothing.getMin(); info.max = m->velocitySmoothing.getMax(); break;
	}
	return info;
}

void MeatbagsPanel::setParamValue(int p, float value, Meatbags* m) {
	switch (p) {
		case 0: m->epsilon        = ofClamp(value, m->epsilon.getMin(),         m->epsilon.getMax());         break;
		case 1: m->minPoints      = ofClamp(value, m->minPoints.getMin(),       m->minPoints.getMax());       break;
		case 2: m->blobPersistence  = ofClamp(value, m->blobPersistence.getMin(),  m->blobPersistence.getMax());  break;
		case 3: m->velocitySmoothing = ofClamp(value, m->velocitySmoothing.getMin(), m->velocitySmoothing.getMax()); break;
	}
}

// -----------------------------------------------------------------------------
// Draw
// -----------------------------------------------------------------------------

void MeatbagsPanel::draw(const std::vector<Meatbags*>& meatbags) {
	int n = meatbags.size();

	// grow state vectors if new instances added
	while (clustererDropdownOpen.size() < n) clustererDropdownOpen.push_back(false);
	while (clustererHoveredItem.size() < n) clustererHoveredItem.push_back("");

	ofFill();
	ofSetColor(backgroundColor);
	ofDrawRectangle(x, y, width, totalHeight(n));

	drawHeader();

	if (!isCollapsed) {
		for (int i = 0; i < n; i++) {
			drawInstance(i, meatbags[i]);
		}
	}
}

void MeatbagsPanel::drawHeader() {
	ofFill();
	ofSetColor(headerColor);
	ofDrawRectangle(panelHeaderRect());
	ofSetColor(headerTextColor);
	ofDrawBitmapString("meatbags", x + padding, y + 14);
	string indicator = isCollapsed ? "+" : "-";
	float indW = indicator.length() * 8;
	ofDrawBitmapString(indicator, x + width - indW - padding, y + 14);
}

void MeatbagsPanel::drawInstance(int i, Meatbags* m) {
	drawInstanceHeader(i);
	drawClustererDropdown(i, m);
	for (int p = 0; p < NUM_PARAMS; p++) {
		drawParamRow(i, p, m);
	}
}

void MeatbagsPanel::drawInstanceHeader(int i) {
	ofRectangle rect = instanceHeaderRect(i);
	ofFill();
	ofSetColor(instanceHeaderColor);
	ofDrawRectangle(rect);
	ofSetColor(dimTextColor);
	ofDrawBitmapString("meatbags " + to_string(i + 1), rect.x + padding, rect.y + 11);
	// no collapse indicator on instances — whole panel collapses
}

void MeatbagsPanel::drawClustererDropdown(int i, Meatbags* m) {
	ofRectangle row = clustererRowRect(i);
	bool open = clustererDropdownOpen[i];
	string current = m->getClustererName();

	// row background
	ofFill();
	ofSetColor(dropdownRowColor);
	ofDrawRectangle(row);

	ofSetColor(ofColor(80, 80, 80));
	ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());

	// label left
	ofSetColor(textColor);
	ofDrawBitmapString("clusterer", row.x + padding, row.y + 13);

	// current value right-aligned
	float valW = current.length() * 8;
	ofSetColor(dimTextColor);
	ofDrawBitmapString(current, row.getRight() - valW - padding, row.y + 13);

	if (open) {
		const auto& names = clustererNames();
		for (int j = 0; j < names.size(); j++) {
			ofRectangle itemRow = clustererItemRect(i, j);
			bool hovered = (clustererHoveredItem[i] == names[j]);

			ofFill();
			ofSetColor(hovered ? dropdownItemHoverColor : dropdownItemColor);
			ofDrawRectangle(itemRow);

			if (names[j] == current) {
				ofSetColor(accentColor);
				ofDrawBitmapString("*", itemRow.x + padding, itemRow.y + 13);
			}

			ofSetColor(hovered ? textColor : dimTextColor);
			ofDrawBitmapString(names[j], itemRow.x + padding + 12, itemRow.y + 13);

			ofSetColor(ofColor(50, 50, 50));
			ofDrawLine(itemRow.x, itemRow.getBottom(), itemRow.getRight(), itemRow.getBottom());
		}
	}
}

void MeatbagsPanel::drawParamRow(int i, int p, Meatbags* m) {
	ParamInfo info    = getParamInfo(p, m);
	ofRectangle slRect = sliderRect(i, p);
	ofRectangle nbRect = numberBoxRect(i, p);
	ofRectangle row    = paramRowRect(i, p);

	bool rowActive   = isDragging && dragInstance == i && dragParam == p;
	bool rowEditing  = isEditing  && editInstance == i && editParam  == p;

	ofFill();
	ofSetColor(p % 2 == 0 ? rowColor : rowAltColor);
	ofDrawRectangle(row);

	drawSlider(slRect, info.value, info.min, info.max, rowActive);

	ofSetColor(textColor);
	ofDrawBitmapString(info.label, slRect.x + padding, slRect.y + 13);

	drawNumberBox(nbRect, info.value, rowEditing);

	ofSetColor(ofColor(50, 50, 50));
	ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());
}

void MeatbagsPanel::drawSlider(const ofRectangle& rect, float value, float min, float max, bool active) {
	float t = (max > min) ? (value - min) / (max - min) : 0.0f;

	ofFill();
	ofSetColor(sliderTrackColor);
	ofDrawRectangle(rect);

	ofSetColor(active ? ofColor(180, 140, 180, 210) : sliderFillColor);
	ofDrawRectangle(rect.x, rect.y, rect.width * t, rect.height);
}

void MeatbagsPanel::drawNumberBox(const ofRectangle& rect, float value, bool editing) {
	ofFill();
	ofSetColor(editing ? numberBoxActiveColor : numberBoxColor);
	ofDrawRectangle(rect);

	ofNoFill();
	ofSetColor(editing ? accentColor : ofColor(70, 70, 70));
	ofDrawRectangle(rect);

	string display = editing ? editText + "|" : ofToString(value, 2);
	float textW = display.length() * 8;
	ofFill();
	ofSetColor(editing ? textColor : dimTextColor);
	ofDrawBitmapString(display, rect.getRight() - textW - padding, rect.y + 13);
}

// -----------------------------------------------------------------------------
// Mouse
// -----------------------------------------------------------------------------

bool MeatbagsPanel::onMousePressed(ofMouseEventArgs& args, const std::vector<Meatbags*>& meatbags) {
	ofPoint mouse(args.x, args.y);
	int n = meatbags.size();

	while (clustererDropdownOpen.size() < n) clustererDropdownOpen.push_back(false);
	while (clustererHoveredItem.size() < n) clustererHoveredItem.push_back("");

	// panel header — toggle collapse
	if (panelHeaderRect().inside(mouse)) {
		isCollapsed = !isCollapsed;
		isEditing   = false;
		return true;
	}

	if (isCollapsed) return false;

	for (int i = 0; i < n; i++) {
		// clusterer dropdown row — toggle open/close
		if (clustererRowRect(i).inside(mouse)) {
			clustererDropdownOpen[i] = !clustererDropdownOpen[i];
			isEditing = false;
			return true;
		}

		// clusterer dropdown items
		if (clustererDropdownOpen[i]) {
			const auto& names = clustererNames();
			for (int j = 0; j < names.size(); j++) {
				if (clustererItemRect(i, j).inside(mouse)) {
					// swap clusterer
					if (names[j] == "DBSCAN") {
						meatbags[i]->setClusterer(std::make_unique<DBSCANClusterer>());
					} else if (names[j] == "Euclidean") {
						meatbags[i]->setClusterer(std::make_unique<EuclideanClusterer>());
					}
					clustererDropdownOpen[i] = false;
					return true;
				}
			}
			// click outside items closes dropdown
			clustererDropdownOpen[i] = false;
			return true;
		}

		// number box
		for (int p = 0; p < NUM_PARAMS; p++) {
			if (numberBoxRect(i, p).inside(mouse)) {
				isEditing    = true;
				editInstance = i;
				editParam    = p;
				ParamInfo info = getParamInfo(p, meatbags[i]);
				editText     = ofToString(info.value, 2);
				isDragging   = false;
				return true;
			}

			// slider
			if (sliderRect(i, p).inside(mouse)) {
				isEditing  = false;
				isDragging = true;
				dragInstance = i;
				dragParam    = p;
				ParamInfo info = getParamInfo(p, meatbags[i]);
				float t = ofClamp((mouse.x - sliderRect(i, p).x) / sliderRect(i, p).width, 0.0f, 1.0f);
				setParamValue(p, info.min + t * (info.max - info.min), meatbags[i]);
				return true;
			}
		}
	}

	// click outside while editing — confirm
	if (isEditing) {
		try {
			setParamValue(editParam, std::stof(editText), meatbags[editInstance]);
		} catch (...) {}
		isEditing = false;
	}

	return false;
}

void MeatbagsPanel::onMouseDragged(ofMouseEventArgs& args, const std::vector<Meatbags*>& meatbags) {
	if (!isDragging) return;
	if (dragInstance < 0 || dragInstance >= meatbags.size()) return;

	ofPoint mouse(args.x, args.y);
	ofRectangle slRect = sliderRect(dragInstance, dragParam);
	ParamInfo info = getParamInfo(dragParam, meatbags[dragInstance]);

	float t = ofClamp((mouse.x - slRect.x) / slRect.width, 0.0f, 1.0f);
	setParamValue(dragParam, info.min + t * (info.max - info.min), meatbags[dragInstance]);
}

bool MeatbagsPanel::onMouseReleased(ofMouseEventArgs& args) {
	if (isDragging) {
		isDragging   = false;
		dragInstance = -1;
		dragParam    = -1;
		return true;
	}
	return false;
}

void MeatbagsPanel::onMouseMoved(ofMouseEventArgs& args, const std::vector<Meatbags*>& meatbags) {
	ofPoint mouse(args.x, args.y);
	int n = meatbags.size();

	while (clustererDropdownOpen.size() < n) clustererDropdownOpen.push_back(false);
	while (clustererHoveredItem.size() < n) clustererHoveredItem.push_back("");

	for (int i = 0; i < n; i++) {
		clustererHoveredItem[i] = "";
		if (clustererDropdownOpen[i]) {
			const auto& names = clustererNames();
			for (int j = 0; j < names.size(); j++) {
				if (clustererItemRect(i, j).inside(mouse)) {
					clustererHoveredItem[i] = names[j];
					break;
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Keyboard
// -----------------------------------------------------------------------------

bool MeatbagsPanel::onKeyPressed(ofKeyEventArgs& args, const std::vector<Meatbags*>& meatbags) {
	if (!isEditing) return false;

	int key = args.key;

	if (key == OF_KEY_RETURN || key == OF_KEY_TAB) {
		try {
			setParamValue(editParam, std::stof(editText), meatbags[editInstance]);
		} catch (...) {}
		isEditing = false;
		return true;
	}

	if (key == OF_KEY_ESC) {
		isEditing = false;
		return true;
	}

	if (key == OF_KEY_BACKSPACE) {
		if (!editText.empty()) editText.pop_back();
		return true;
	}

	char c = (char)key;
	if (std::isdigit(c) || c == '-' || c == '.') {
		editText += c;
		return true;
	}

	return false;
}

// -----------------------------------------------------------------------------
// Save / Load
// -----------------------------------------------------------------------------

void MeatbagsPanel::saveTo(ofJson& config) {
	config["meatbags_panel"]["collapsed"] = isCollapsed;
}

void MeatbagsPanel::loadFrom(ofJson& config) {
	if (!config.contains("meatbags_panel")) return;
	ofJson g = config["meatbags_panel"];
	if (g.contains("collapsed")) isCollapsed = g["collapsed"].get<bool>();
}
