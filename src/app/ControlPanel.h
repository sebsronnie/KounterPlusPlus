// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ControlPanel.h
//  Author:  Kelvin Maritim
//  Purpose: The ofxGui panel with all sliders, toggles and buttons.
//           It copies its values into a DetectionSettings struct every frame
//           (applyTo) and can be refreshed from one (readFrom) when a key
//           shortcut or a colour pick changed the settings behind its back.
// ---------------------------------------------------------------------------
#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include <functional>

#include "../vision/DetectionSettings.h"

class ControlPanel {
public:
	ControlPanel();
	~ControlPanel();

	void setup(float x, float y);
	void draw();

	void applyTo(DetectionSettings& s) const;      // GUI -> settings
	void readFrom(const DetectionSettings& s);      // settings -> GUI

	bool isMouseOver(int x, int y) const;
	ofRectangle getShape() const { return gui.getShape(); }

	// ofApp plugs its own functions in here. The panel does not know what
	// "load an image" means; it only knows the button was pressed.
	std::function<void()> onLoadImage;
	std::function<void()> onUseCamera;
	std::function<void()> onLoadVideo;
	std::function<void()> onClearClasses;
	std::function<void()> onResetCounter;
	std::function<void()> onSaveScreenshot;
	std::function<void()> onExportCsv;

private:
	ofxPanel gui;

	// --- buttons ---
	ofxButton loadImageButton, cameraButton, videoButton;
	ofxButton clearClassesButton, resetCounterButton, screenshotButton, csvButton;

	// --- parameters, grouped the way they appear on screen ---
	ofParameterGroup   segmentation;
	ofParameter<int>   mode;
	ofParameter<bool>  forceGrayscale, invert, autoInvert;
	ofParameter<int>   minContrast, threshold, adaptiveBlockSize, adaptiveOffset;
	ofParameter<float> tolerance;

	ofParameterGroup   cleanup;
	ofParameter<int>   blurSize, openIterations, closeIterations;
	ofParameter<bool>  fillHoles, splitTouching;
	ofParameter<float> splitStrength;

	ofParameterGroup   filtering;
	ofParameter<float> minArea, maxArea;

	ofParameterGroup   classification;
	ofParameter<bool>  useColour, useShape, autoGroup;
	ofParameter<int>   groupCount;
	ofParameter<float> maxMatchDistance;

	ofParameterGroup   video;
	ofParameter<float> trackMaxDistance;
	ofParameter<int>   trackMaxMissed, processWidth;

	// button handlers just forward to the std::function callbacks
	void loadImagePressed()      { if (onLoadImage)      onLoadImage(); }
	void cameraPressed()         { if (onUseCamera)      onUseCamera(); }
	void videoPressed()          { if (onLoadVideo)      onLoadVideo(); }
	void clearClassesPressed()   { if (onClearClasses)   onClearClasses(); }
	void resetCounterPressed()   { if (onResetCounter)   onResetCounter(); }
	void screenshotPressed()     { if (onSaveScreenshot) onSaveScreenshot(); }
	void csvPressed()            { if (onExportCsv)      onExportCsv(); }
};
