#include "ofMain.h"
#include "ofApp.h"

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "Version.lib")

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	ofSetupOpenGL(1200, 800, OF_WINDOW);
	ofRunApp(new ofApp());
	return 0;
}
#else

int main() {
	ofGLWindowSettings settings;
	settings.setSize(1200, 800);
	settings.windowMode = OF_WINDOW;

	auto window = ofCreateWindow(settings);

	ofRunApp(window, make_shared<ofApp>());
	ofRunMainLoop();
}
#endif
