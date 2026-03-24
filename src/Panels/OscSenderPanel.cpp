//
//  OscSenderPanel.cpp
//  meatbags

#include "OscSenderPanel.hpp"

OscSenderPanel::OscSenderPanel() {
	x = 0; y = 0; width = 210;

	isCollapsed = false;

	isEditingIP    = false;
	editIPInstance = -1;
	editIPText     = "";

	isEditingPort    = false;
	editPortInstance = -1;
	editPortText     = "";

	headerHeight         = 20;
	instanceHeaderHeight = 16;
	rowHeight            = 18;
	checkboxSize         = 10;
	padding              = 6;
	indent               = 8;
	portBoxW             = 60;

	backgroundColor      = ofColor(0, 0, 0, 220);
	headerColor          = ofColor::thistle;
	headerTextColor      = ofColor::black;
	instanceHeaderColor  = ofColor(40, 40, 40, 255);
	rowColor             = ofColor(20, 20, 20, 240);
	rowAltColor          = ofColor(28, 28, 28, 240);
	controlRowColor      = ofColor(30, 20, 30, 240);
	numberBoxColor       = ofColor(35, 35, 35, 255);
	numberBoxActiveColor = ofColor(70, 50, 70, 255);
	textColor            = ofColor::white;
	dimTextColor         = ofColor(160, 160, 160);
	accentColor          = ofColor::thistle;
}

// -----------------------------------------------------------------------------
// Layout
// -----------------------------------------------------------------------------

int OscSenderPanel::numContentRows() {
	// IP row + port row + send blobs + send filters + send logs
	return 5;
}

int OscSenderPanel::instanceHeight(int i) {
	if (i < instanceCollapsed.size() && instanceCollapsed[i]) return instanceHeaderHeight;
	return instanceHeaderHeight + numContentRows() * rowHeight;
}

int OscSenderPanel::instanceYOffset(int i, const std::vector<OscSender*>& senders) {
	int offset = y + headerHeight;
	for (int j = 0; j < i; j++) offset += instanceHeight(j);
	return offset;
}

int OscSenderPanel::totalHeight(int n) {
	if (isCollapsed) return headerHeight;
	int h = headerHeight;
	for (int i = 0; i < n; i++) h += instanceHeight(i);
	return h;
}

ofRectangle OscSenderPanel::panelHeaderRect() {
	return ofRectangle(x, y, width, headerHeight);
}

ofRectangle OscSenderPanel::instanceHeaderRect(int i, const std::vector<OscSender*>& senders) {
	return ofRectangle(x, instanceYOffset(i, senders), width, instanceHeaderHeight);
}

ofRectangle OscSenderPanel::rowRect(int i, int row, const std::vector<OscSender*>& senders) {
	int rowY = instanceYOffset(i, senders) + instanceHeaderHeight + row * rowHeight;
	return ofRectangle(x + indent, rowY, width - indent, rowHeight);
}

ofRectangle OscSenderPanel::portBoxRect(int i, const std::vector<OscSender*>& senders) {
	ofRectangle row = rowRect(i, 1, senders);  // port is row 1
	return ofRectangle(x + width - portBoxW, row.y, portBoxW, row.height);
}

// -----------------------------------------------------------------------------
// Draw
// -----------------------------------------------------------------------------

void OscSenderPanel::draw(const std::vector<OscSender*>& senders) {
	int n = senders.size();
	while (instanceCollapsed.size() < n) instanceCollapsed.push_back(false);

	int bgHeight = (isCollapsed || n == 0) ? headerHeight
				 : std::min(totalHeight(n), (int)(ofGetHeight() - y - 20));
	ofFill();
	ofSetColor(backgroundColor);
	ofDrawRectangle(x, y, width, bgHeight);

	drawHeader();
	if (isCollapsed || n == 0) return;

	for (int i = 0; i < n; i++) {
		drawInstance(i, senders[i], senders);
	}
}

void OscSenderPanel::drawHeader() {
	ofFill();
	ofSetColor(headerColor);
	ofDrawRectangle(panelHeaderRect());
	ofSetColor(headerTextColor);
	ofDrawBitmapString("osc senders", x + padding, y + 14);
	string ind = isCollapsed ? "+" : "-";
	ofDrawBitmapString(ind, x + width - ind.length() * 8 - padding, y + 14);
}

void OscSenderPanel::drawInstance(int i, OscSender* s, const std::vector<OscSender*>& senders) {
	int baseY    = instanceYOffset(i, senders);
	bool collapsed = (i < instanceCollapsed.size()) && instanceCollapsed[i];

	// instance header
	ofFill();
	ofSetColor(instanceHeaderColor);
	ofDrawRectangle(x, baseY, width, instanceHeaderHeight);
	ofSetColor(dimTextColor);
	ofDrawBitmapString("osc sender " + to_string(s->index), x + padding, baseY + 11);
	string ind = collapsed ? "+" : "-";
	ofDrawBitmapString(ind, x + width - ind.length() * 8 - padding, baseY + 11);

	if (collapsed) return;

	float rowRight = (float)(x + width);

	// row 0: IP address
	{
		ofRectangle row = rowRect(i, 0, senders);
		bool editing = isEditingIP && editIPInstance == i;
		string display = editing ? editIPText + "|" : s->oscSenderAddress.get();

		ofFill();
		ofSetColor(editing ? numberBoxActiveColor : rowColor);
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

		ofSetColor(ofColor(50, 50, 50));
		ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());
	}

	// row 1: port
	{
		ofRectangle row = rowRect(i, 1, senders);
		ofRectangle nb  = portBoxRect(i, senders);
		bool editing = isEditingPort && editPortInstance == i;
		string display = editing ? editPortText + "|" : ofToString(s->oscSenderPort.get());

		ofFill();
		ofSetColor(rowAltColor);
		ofDrawRectangle(row);

		ofSetColor(textColor);
		ofDrawBitmapString("port", row.x + padding, row.y + 13);

		ofFill();
		ofSetColor(editing ? numberBoxActiveColor : numberBoxColor);
		ofDrawRectangle(nb);
		ofNoFill();
		ofSetColor(editing ? accentColor : ofColor(70, 70, 70));
		ofDrawRectangle(nb);
		ofFill();
		float portW = display.length() * 8;
		ofSetColor(editing ? textColor : dimTextColor);
		ofDrawBitmapString(display, rowRight - portW - padding, row.y + 13);

		ofSetColor(ofColor(50, 50, 50));
		ofDrawLine(row.x, row.getBottom(), row.getRight(), row.getBottom());
	}

	// rows 2-4: checkboxes
	struct CheckRow { string label; bool value; };
	CheckRow checks[3] = {
		{ "send blobs",   s->sendBlobsActive   },
		{ "send filters", s->sendFiltersActive },
		{ "send logs",    s->sendLogsActive    }
	};

	for (int r = 0; r < 3; r++) {
		ofRectangle row = rowRect(i, 2 + r, senders);

		ofFill();
		ofSetColor(r % 2 == 0 ? controlRowColor : rowColor);
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
	}
}

// -----------------------------------------------------------------------------
// Mouse
// -----------------------------------------------------------------------------

bool OscSenderPanel::onMousePressed(ofMouseEventArgs& args, const std::vector<OscSender*>& senders) {
	ofPoint mouse(args.x, args.y);
	int n = senders.size();
	while (instanceCollapsed.size() < n) instanceCollapsed.push_back(false);

	if (panelHeaderRect().inside(mouse)) {
		isCollapsed   = !isCollapsed;
		isEditingIP   = isEditingPort = false;
		return true;
	}
	if (isCollapsed) return false;

	for (int i = 0; i < n; i++) {
		// instance header — toggle collapse
		ofRectangle hdr = instanceHeaderRect(i, senders);
		if (hdr.inside(mouse)) {
			instanceCollapsed[i] = !instanceCollapsed[i];
			isEditingIP = isEditingPort = false;
			return true;
		}
		if (instanceCollapsed[i]) continue;

		// row 0: IP
		{
			ofRectangle row = rowRect(i, 0, senders);
			if (row.inside(mouse)) {
				isEditingIP    = true;
				editIPInstance = i;
				editIPText     = senders[i]->oscSenderAddress.get();
				isEditingPort  = false;
				return true;
			}
		}

		// row 1: port box
		{
			ofRectangle nb = portBoxRect(i, senders);
			if (nb.inside(mouse)) {
				isEditingPort    = true;
				editPortInstance = i;
				editPortText     = ofToString(senders[i]->oscSenderPort.get());
				isEditingIP      = false;
				return true;
			}
		}

		// rows 2-4: checkboxes
		ofParameter<bool>* checks[3] = {
			&senders[i]->sendBlobsActive,
			&senders[i]->sendFiltersActive,
			&senders[i]->sendLogsActive
		};
		for (int r = 0; r < 3; r++) {
			ofRectangle row = rowRect(i, 2 + r, senders);
			if (row.inside(mouse)) {
				*checks[r] = !checks[r]->get();
				return true;
			}
		}
	}

	// click outside — confirm edits
	if (isEditingIP) {
		senders[editIPInstance]->oscSenderAddress = editIPText;
		isEditingIP = false;
	}
	if (isEditingPort) {
		try {
			senders[editPortInstance]->oscSenderPort = std::stoi(editPortText);
		} catch (...) {}
		isEditingPort = false;
	}

	return false;
}

void OscSenderPanel::onMouseMoved(ofMouseEventArgs& args, const std::vector<OscSender*>& senders) {
	// no hover state needed currently
}

// -----------------------------------------------------------------------------
// Keyboard
// -----------------------------------------------------------------------------

bool OscSenderPanel::onKeyPressed(ofKeyEventArgs& args, const std::vector<OscSender*>& senders) {
	int key = args.key;

	if (isEditingIP) {
		if (key == OF_KEY_RETURN || key == OF_KEY_TAB) {
			senders[editIPInstance]->oscSenderAddress = editIPText;
			isEditingIP = false;
			return true;
		}
		if (key == OF_KEY_ESC)       { isEditingIP = false; return true; }
		if (key == OF_KEY_BACKSPACE) { if (!editIPText.empty()) editIPText.pop_back(); return true; }
		char c = (char)key;
		if (std::isdigit(c) || c == '.' || std::isalpha(c)) { editIPText += c; return true; }
		return false;
	}

	if (isEditingPort) {
		if (key == OF_KEY_RETURN || key == OF_KEY_TAB) {
			try { senders[editPortInstance]->oscSenderPort = std::stoi(editPortText); } catch (...) {}
			isEditingPort = false;
			return true;
		}
		if (key == OF_KEY_ESC)       { isEditingPort = false; return true; }
		if (key == OF_KEY_BACKSPACE) { if (!editPortText.empty()) editPortText.pop_back(); return true; }
		char c = (char)key;
		if (std::isdigit(c)) { editPortText += c; return true; }
		return false;
	}

	return false;
}

// -----------------------------------------------------------------------------
// Save / Load
// -----------------------------------------------------------------------------

void OscSenderPanel::saveTo(ofJson& config) {
	config["osc_sender_panel"]["collapsed"] = isCollapsed;

	ofJson instances = ofJson::array();
	for (int i = 0; i < instanceCollapsed.size(); i++) {
		instances.push_back(instanceCollapsed[i]);
	}
	config["osc_sender_panel"]["instance_collapsed"] = instances;
}

void OscSenderPanel::loadFrom(ofJson& config) {
	if (!config.contains("osc_sender_panel")) return;
	ofJson g = config["osc_sender_panel"];

	if (g.contains("collapsed")) isCollapsed = g["collapsed"].get<bool>();

	if (g.contains("instance_collapsed")) {
		instanceCollapsed.clear();
		for (auto& val : g["instance_collapsed"]) {
			instanceCollapsed.push_back(val.get<bool>());
		}
	}
}
