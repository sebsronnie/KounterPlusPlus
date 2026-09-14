// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    main.cpp
//  Author:  Ronald K. Sebuhinja
//  Purpose: Creates the window and starts the app (openFrameworks 0.12 style).
// -----------------------------------------------------------------------------
#include "ofMain.h"
#include "ofApp.h"

int main() {
	ofGLWindowSettings settings;
	settings.setSize(1280, 800);
	settings.windowMode = OF_WINDOW;

	auto window = ofCreateWindow(settings);

	// ofApp lives on the heap inside a shared_ptr; openFrameworks keeps it
	// alive for the whole run and destroys it when the window closes.
	ofRunApp(window, std::make_shared<ofApp>());
	ofRunMainLoop();
	return 0;
}
