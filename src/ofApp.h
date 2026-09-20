// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ofApp.h
//  Author:  Ronald K. Sebuhinja (shared file: change only by agreement)
//  Purpose: Composition root. Owns the source, the detector, the panel and
//           the overlay, and forwards events between them. Keep it thin.
// -----------------------------------------------------------------------------
#pragma once

#include "ofMain.h"
#include <memory>
#include <string>

#include "app/ControlPanel.h"
#include "app/ResultOverlay.h"
#include "sources/FrameSource.h"
#include "vision/ObjectDetector.h"

class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void exit() override;

	void keyPressed(int key) override;
	void mousePressed(int x, int y, int button) override;
	void dragEvent(ofDragInfo dragInfo) override;
	void windowResized(int w, int h) override;

private:
	// --- source switching ---
	void openImage(const std::string& path);
	void openCamera();
	void openVideo(const std::string& path);
	void openFile(const std::string& path);     // decides image vs video by extension
	void chooseImageDialog();
	void chooseVideoDialog();
	void setSource(std::unique_ptr<FrameSource> next);

	// --- processing ---
	void runDetection();
	void handleImageClick(int ix, int iy);
	void finishTeaching();
	void cancelTeaching();
	void resetSettings();

	// --- utilities ---
	void saveScreenshot();
	void exportCsv();
	void say(const std::string& msg);           // status line in the HUD
	void layout();

	// The four collaborators (composition: ofApp owns them all).
	std::unique_ptr<FrameSource> source;        // heap, polymorphic
	ObjectDetector               detector;      // member, lives as long as the app
	ControlPanel                 panel;
	ResultOverlay                overlay;

	DetectionSettings settings, lastSettings;
	DetectionResult   result;

	cv::Mat   frame;                            // wraps the source pixels, no copy
	ofTexture frameTexture, maskTexture;

	ViewMode view      = ViewMode::Overlay;
	bool     showHelp  = false;
	bool     needsRun  = false;

	// teach-by-click state
	bool        teachMode        = false;
	int         teachObjectIndex = -1;          // which object is being named
	std::string labelBuffer;                    // the name being typed
	bool        typing           = false;

	std::string message;
	float       messageUntil = 0.0f;
};
