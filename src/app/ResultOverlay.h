// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ResultOverlay.h
//  Author:  Kelvin Maritim
//  Purpose: Draws the picture, the found objects (outline, box, label) and
//           the heads-up display with counts and status. Also converts
//           mouse positions back into picture coordinates.
// ---------------------------------------------------------------------------
#pragma once
#include "ofMain.h"
#include <string>
#include <vector>

#include "../vision/ObjectClassifier.h"
#include "../vision/ObjectDetector.h"

enum class ViewMode { Original = 0, Mask, Overlay };

// Everything the HUD needs that is not inside DetectionResult.
struct HudInfo {
	std::string sourceName;
	std::string modeName;
	bool        isVideo   = false;
	bool        teachMode = false;
	std::string prompt;        // e.g. "Name for object 3: oran_"
	std::string message;       // short status line, e.g. "Saved screenshot"
	bool        showHelp  = false;
};

class ResultOverlay {
public:
	void setup();

	// The screen area the picture may use (everything right of the panel).
	void setViewport(const ofRectangle& area);

	void draw(const ofTexture& frame, const ofTexture& mask, const DetectionResult& result,
	          ViewMode view, const std::vector<ObjectClass>& classes, const HudInfo& hud);

	// Mouse position -> picture pixel. Returns false when outside the picture.
	bool screenToImage(int sx, int sy, int& ix, int& iy) const;

	static ofColor colourForClass(int classIndex);

private:
	void fitImage(float imageW, float imageH);
	void drawObjects(const DetectionResult& r);
	void drawHud(const DetectionResult& r, const std::vector<ObjectClass>& classes, const HudInfo& hud);
	void drawHelp();
	void text(const std::string& s, float x, float y);
	float textWidth(const std::string& s);

	ofRectangle    viewport;
	ofRectangle    imageRect;   // where the picture actually lands on screen
	float          scale = 1.0f;
	ofTrueTypeFont font;
	bool           fontLoaded = false;
};
