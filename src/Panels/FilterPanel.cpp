//
//  FilterPanel.cpp
//  meatbags

#include "FilterPanel.hpp"

const std::vector<std::string>& FilterPanel::filterTypeNames() {
	static std::vector<std::string> names = { "Quad", "Ellipse" };
	return names;
}

string FilterPanel::currentTypeName(Filter* f) {
	if (dynamic_cast<QuadFilter*>(f))    return "Quad";
	if (dynamic_cast<EllipseFilter*>(f)) return "Ellipse";
	return "Quad";
}

FilterPanel::FilterPanel() {
	x = 0; y = 0; width = 210;
	scrollY = 0.0f;

	isCollapsed          = false;
	isScrollbarDragging  = false;
	scrollbarDragStartY  = 0.0f;
	scrollbarDragStartScrollY = 0.0f;

	headerHeight         = 20;
	instanceHeaderHeight = 16;
	rowHeight            = 18;
	checkboxSize         = 10;
	padding              = 6;
	indent               = 8;
	bottomMargin         = 20;
	scrollbarWidth       = 4;

	backgroundColor        = ofColor(0, 0, 0, 220);
	headerColor            = ofColor::thistle;
	headerTextColor        = ofColor::black;
	instanceHeaderColor    = ofColor(40, 40, 40, 255);
	rowColor               = ofColor(20, 20, 20, 240);
	rowAltColor            = ofColor(28, 28, 28, 240);
	controlRowColor        = ofColor(30, 20, 30, 240);
	dropdownItemColor      = ofColor(10, 10, 10, 240);
	dropdownItemHoverColor = ofColor(60, 60, 60, 240);
	scrollbarColor         = ofColor(30, 30, 30, 180);
	scrollbarThumbColor    = ofColor(120, 100, 120, 220);
	textColor              = ofColor::white;
	dimTextColor           = ofColor(160, 160, 160);
	accentColor            = ofColor::thistle;
}

// -----------------------------------------------------------------------------
// Layout
// -----------------------------------------------------------------------------

int FilterPanel::availableHeight() {
	return ofGetHeight() - y - bottomMargin;
}

int FilterPanel::instanceHeight(int i) {
	if (i < instanceCollapsed.size() && instanceCollapsed[i]) return instanceHeaderHeight;
	bool dropOpen = i < typeDropdownOpen.size() && typeDropdownOpen[i];
	int rows = 3; // shape + mask + normalized
	if (dropOpen) rows += filterTypeNames().size();
	return instanceHeaderHeight + rows * rowHeight;
}

int FilterPanel::instanceYOffset(int i, const std::vector<Filter*>& filters) {
	int offset = y + headerHeight - (int)scrollY;
	for (int j = 0; j < i; j++) offset += instanceHeight(j);
	return offset;
}

int FilterPanel::totalContentHeight(const std::vector<Filter*>& filters) {
	int h = 0;
	for (int i = 0; i < filters.size(); i++) h += instanceHeight(i);
	return h;
}

int FilterPanel::totalHeight(int n) {
	if (isCollapsed) return headerHeight;
	int h = headerHeight;
	for (int i = 0; i < n; i++) h += instanceHeight(i);
	return h;
}

bool FilterPanel::isScrollable(const std::vector<Filter*>& filters) {
	return totalContentHeight(filters) > availableHeight() - headerHeight;
}

void FilterPanel::clampScroll(const std::vector<Filter*>& filters) {
	float maxScroll = (float)(totalContentHeight(filters) - (availableHeight() - headerHeight));
	scrollY = ofClamp(scrollY, 0.0f, std::max(0.0f, maxScroll));
}

ofRectangle FilterPanel::panelHeaderRect() {
	return ofRectangle(x, y, width, headerHeight);
}

ofRectangle FilterPanel::scrollbarTrackRect() {
	return ofRectangle(x + width - scrollbarWidth, y + headerHeight, scrollbarWidth, availableHeight() - headerHeight);
}

ofRectangle FilterPanel::scrollbarThumbRect(const std::vector<Filter*>& filters) {
	ofRectangle track = scrollbarTrackRect();
	int content = totalContentHeight(filters);
	int avail   = availableHeight() - headerHeight;
	if (content <= 0) return track;
	float thumbRatio = ofClamp((float)avail / content, 0.05f, 1.0f);
	float thumbH     = track.height * thumbRatio;
	float maxScroll  = (float)(content - avail);
	float thumbY     = (maxScroll > 0) ? track.y + (scrollY / maxScroll) * (track.height - thumbH) : track.y;
	return ofRectangle(track.x, thumbY, track.width, thumbH);
}

// -----------------------------------------------------------------------------
// Draw
// -----------------------------------------------------------------------------

void FilterPanel::draw(const std::vector<Filter*>& filters) {
	int n = filters.size();
	while (instanceCollapsed.size() < n) instanceCollapsed.push_back(false);
	while (typeDropdownOpen.size() < n)  typeDropdownOpen.push_back(false);
	while (typeHoveredItem.size() < n)   typeHoveredItem.push_back("");

	if (!isCollapsed) clampScroll(filters);

	int bgHeight = (isCollapsed || n == 0) ? headerHeight
				 : std::min(totalContentHeight(filters) + headerHeight, availableHeight());
	ofFill();
	ofSetColor(backgroundColor);
	ofDrawRectangle(x, y, width, bgHeight);

	drawHeader();
	if (isCollapsed || n == 0) return;

	int contentAreaY = y + headerHeight;
	int contentAreaH = availableHeight() - headerHeight;
	float s = (float)ofGetCurrentRenderer()->getViewportWidth() / (float)ofGetWidth();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int)(x * s),
			  (int)((ofGetHeight() - contentAreaY - contentAreaH) * s),
			  (int)(width * s),
			  (int)(contentAreaH * s));

	for (int i = 0; i < n; i++) {
		drawInstance(i, filters[i], filters);
	}

	glDisable(GL_SCISSOR_TEST);

	if (isScrollable(filters)) drawScrollbar(filters);
}

void FilterPanel::drawHeader() {
	ofFill();
	ofSetColor(headerColor);
	ofDrawRectangle(panelHeaderRect());
	ofSetColor(headerTextColor);
	ofDrawBitmapString("filters", x + padding, y + 14);
	string ind = isCollapsed ? "+" : "-";
	ofDrawBitmapString(ind, x + width - (int)ind.length() * 8 - padding, y + 14);
}

void FilterPanel::drawScrollbar(const std::vector<Filter*>& filters) {
	ofRectangle track = scrollbarTrackRect();
	ofFill();
	ofSetColor(scrollbarColor);
	ofDrawRectangle(track);
	ofSetColor(scrollbarThumbColor);
	ofDrawRectangle(scrollbarThumbRect(filters));
}

void FilterPanel::drawInstance(int i, Filter* f, const std::vector<Filter*>& filters) {
	int baseY    = instanceYOffset(i, filters);
	bool collapsed = (i < instanceCollapsed.size()) && instanceCollapsed[i];
	bool dropOpen  = (i < typeDropdownOpen.size())  && typeDropdownOpen[i];
	string current = currentTypeName(f);
	float rowRight = (float)(x + width);

	// instance header
	ofFill();
	ofSetColor(instanceHeaderColor);
	ofDrawRectangle(x, baseY, width, instanceHeaderHeight);
	ofSetColor(dimTextColor);
	ofDrawBitmapString("filter " + to_string(f->index), x + padding, baseY + 11);
	string ind = collapsed ? "+" : "-";
	ofDrawBitmapString(ind, x + width - (int)ind.length() * 8 - padding, baseY + 11);

	if (collapsed) return;

	int contentY = baseY + instanceHeaderHeight;

	// shape dropdown row
	ofRectangle shapeRow(x + indent, contentY, width - indent, rowHeight);
	ofFill();
	ofSetColor(controlRowColor);
	ofDrawRectangle(shapeRow);
	ofSetColor(ofColor(80, 80, 80));
	ofDrawLine(shapeRow.x, shapeRow.getBottom(), shapeRow.getRight(), shapeRow.getBottom());

	ofSetColor(textColor);
	ofDrawBitmapString("shape", shapeRow.x + padding, shapeRow.y + 13);
	float typeW = current.length() * 8;
	ofSetColor(dimTextColor);
	ofDrawBitmapString(current, rowRight - typeW - padding, shapeRow.y + 13);
	contentY += rowHeight;

	// dropdown items
	if (dropOpen) {
		for (int j = 0; j < filterTypeNames().size(); j++) {
			string name = filterTypeNames()[j];
			ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
			bool hovered = (typeHoveredItem[i] == name);

			ofFill();
			ofSetColor(hovered ? dropdownItemHoverColor : dropdownItemColor);
			ofDrawRectangle(itemRect);

			if (name == current) {
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

	// mask + normalized checkboxes
	struct CheckRow { string label; bool value; };
	CheckRow checks[2] = {
		{ "mask",       f->isMask.get()       },
		{ "normalized", f->isNormalized.get() }
	};

	for (int r = 0; r < 2; r++) {
		ofRectangle row(x + indent, contentY, width - indent, rowHeight);
		ofFill();
		ofSetColor(r % 2 == 0 ? rowColor : rowAltColor);
		ofDrawRectangle(row);

		ofRectangle checkRect(row.x + padding, row.y + (rowHeight - checkboxSize) * 0.5f, checkboxSize, checkboxSize);
		ofNoFill();
		ofSetColor(accentColor);
		ofDrawRectangle(checkRect);

		if (checks[r].value) {
			ofFill();
			ofSetColor(accentColor);
			ofRectangle inner = checkRect;
			inner.scaleFromCenter(0.6f);
			ofDrawRectangle(inner);
		}

		ofFill();
		ofSetColor(textColor);
		ofDrawBitmapString(checks[r].label, checkRect.getRight() + padding, row.y + 13);

		ofSetColor(ofColor(50, 50, 50));
		ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());
		contentY += rowHeight;
	}
}

// -----------------------------------------------------------------------------
// Mouse
// -----------------------------------------------------------------------------

bool FilterPanel::onMousePressed(ofMouseEventArgs& args, const std::vector<Filter*>& filters) {
	ofPoint mouse(args.x, args.y);
	int n = filters.size();
	while (instanceCollapsed.size() < n) instanceCollapsed.push_back(false);
	while (typeDropdownOpen.size() < n)  typeDropdownOpen.push_back(false);
	while (typeHoveredItem.size() < n)   typeHoveredItem.push_back("");

	if (panelHeaderRect().inside(mouse)) {
		isCollapsed = !isCollapsed;
		if (isCollapsed) scrollY = 0.0f;
		return true;
	}
	if (isCollapsed) return false;

	// scrollbar thumb drag
	if (isScrollable(filters) && scrollbarThumbRect(filters).inside(mouse)) {
		isScrollbarDragging       = true;
		scrollbarDragStartY       = mouse.y;
		scrollbarDragStartScrollY = scrollY;
		return true;
	}

	for (int i = 0; i < n; i++) {
		int baseY = instanceYOffset(i, filters);

		ofRectangle hdr(x, baseY, width, instanceHeaderHeight);
		if (hdr.inside(mouse)) {
			instanceCollapsed[i] = !instanceCollapsed[i];
			typeDropdownOpen[i]  = false;
			return true;
		}
		if (instanceCollapsed[i]) continue;

		int contentY = baseY + instanceHeaderHeight;

		// shape dropdown row
		ofRectangle shapeRow(x + indent, contentY, width - indent, rowHeight);
		if (shapeRow.inside(mouse)) {
			typeDropdownOpen[i] = !typeDropdownOpen[i];
			return true;
		}
		contentY += rowHeight;

		// dropdown items
		if (typeDropdownOpen[i]) {
			for (int j = 0; j < filterTypeNames().size(); j++) {
				ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
				if (itemRect.inside(mouse)) {
					typeDropdownOpen[i] = false;
					if (onFilterTypeChanged) onFilterTypeChanged(i, filterTypeNames()[j]);
					return true;
				}
				contentY += rowHeight;
			}
			typeDropdownOpen[i] = false;
			return true;
		}

		// mask
		{
			ofRectangle row(x + indent, contentY, width - indent, rowHeight);
			if (row.inside(mouse)) {
				filters[i]->isMask = !filters[i]->isMask.get();
				return true;
			}
			contentY += rowHeight;
		}

		// normalized
		{
			ofRectangle row(x + indent, contentY, width - indent, rowHeight);
			if (row.inside(mouse)) {
				filters[i]->isNormalized = !filters[i]->isNormalized.get();
				return true;
			}
		}
	}

	return false;
}

bool FilterPanel::onMouseReleased(ofMouseEventArgs& args) {
	if (isScrollbarDragging) {
		isScrollbarDragging = false;
		return true;
	}
	return false;
}

void FilterPanel::onMouseMoved(ofMouseEventArgs& args, const std::vector<Filter*>& filters) {
	ofPoint mouse(args.x, args.y);
	int n = filters.size();
	while (typeDropdownOpen.size() < n) typeDropdownOpen.push_back(false);
	while (typeHoveredItem.size() < n)  typeHoveredItem.push_back("");

	for (int i = 0; i < n; i++) {
		typeHoveredItem[i] = "";
		if (!typeDropdownOpen[i]) continue;

		int contentY = instanceYOffset(i, filters) + instanceHeaderHeight + rowHeight;
		for (int j = 0; j < filterTypeNames().size(); j++) {
			ofRectangle itemRect(x + indent, contentY, width - indent, rowHeight);
			if (itemRect.inside(mouse)) {
				typeHoveredItem[i] = filterTypeNames()[j];
				break;
			}
			contentY += rowHeight;
		}
	}
}

bool FilterPanel::onMouseScrolled(ofMouseEventArgs& args, const std::vector<Filter*>& filters) {
	if (isCollapsed) return false;
	ofRectangle panelArea(x, y, width, availableHeight());
	if (!panelArea.inside(ofPoint(args.x, args.y))) return false;
	if (!isScrollable(filters)) return false;
	scrollY -= args.scrollY * rowHeight * 2;
	clampScroll(filters);
	return true;
}

// -----------------------------------------------------------------------------
// Save / Load
// -----------------------------------------------------------------------------

void FilterPanel::saveTo(ofJson& config) {
	config["filter_panel"]["collapsed"] = isCollapsed;
	config["filter_panel"]["scroll_y"]  = scrollY;

	ofJson instances = ofJson::array();
	for (int i = 0; i < instanceCollapsed.size(); i++) {
		instances.push_back(instanceCollapsed[i]);
	}
	config["filter_panel"]["instance_collapsed"] = instances;
}

void FilterPanel::loadFrom(ofJson& config) {
	if (!config.contains("filter_panel")) return;
	ofJson g = config["filter_panel"];

	if (g.contains("collapsed")) isCollapsed = g["collapsed"].get<bool>();
	if (g.contains("scroll_y"))  scrollY     = g["scroll_y"].get<float>();

	if (g.contains("instance_collapsed")) {
		instanceCollapsed.clear();
		for (auto& val : g["instance_collapsed"]) {
			instanceCollapsed.push_back(val.get<bool>());
		}
	}
}
