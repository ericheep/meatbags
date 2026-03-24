//
//  OscSenderPanel.hpp
//  meatbags

#ifndef OscSenderPanel_hpp
#define OscSenderPanel_hpp

#include "ofMain.h"
#include "OscSender.hpp"
#include <vector>
#include <functional>

class OscSenderPanel {
public:
	OscSenderPanel();

	void draw(const std::vector<OscSender*>& senders);

	bool onMousePressed(ofMouseEventArgs& args, const std::vector<OscSender*>& senders);
	void onMouseMoved(ofMouseEventArgs& args, const std::vector<OscSender*>& senders);
	bool onKeyPressed(ofKeyEventArgs& args, const std::vector<OscSender*>& senders);

	void saveTo(ofJson& config);
	void loadFrom(ofJson& config);

	int totalHeight(int n);

	int x, y, width;

private:
	// -------------------------------------------------------------------------
	// Layout
	// -------------------------------------------------------------------------
	ofRectangle panelHeaderRect();
	ofRectangle instanceHeaderRect(int i, const std::vector<OscSender*>& senders);
	ofRectangle rowRect(int i, int row, const std::vector<OscSender*>& senders);
	ofRectangle portBoxRect(int i, const std::vector<OscSender*>& senders);
	int instanceYOffset(int i, const std::vector<OscSender*>& senders);
	int instanceHeight(int i);
	int numContentRows();  // rows below IP: port + 3 checkboxes = 4

	// -------------------------------------------------------------------------
	// Draw
	// -------------------------------------------------------------------------
	void drawHeader();
	void drawInstance(int i, OscSender* s, const std::vector<OscSender*>& senders);

	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------
	bool isCollapsed;
	std::vector<bool> instanceCollapsed;

	// IP editing
	bool        isEditingIP;
	int         editIPInstance;
	std::string editIPText;

	// Port editing
	bool        isEditingPort;
	int         editPortInstance;
	std::string editPortText;

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
	ofColor numberBoxColor;
	ofColor numberBoxActiveColor;
	ofColor textColor;
	ofColor dimTextColor;
	ofColor accentColor;

	int headerHeight;
	int instanceHeaderHeight;
	int rowHeight;
	int checkboxSize;
	int padding;
	int indent;
	int portBoxW;
};

#endif /* OscSenderPanel_hpp */
