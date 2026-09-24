// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ResultOverlay.cpp
//  Author:  Kelvin Maritim
//  Purpose: Drawing of results and HUD.
// ---------------------------------------------------------------------------
#include <algorithm>
#include "ResultOverlay.h"

void ResultOverlay::setup() {
	// OF_TTF_SANS asks the operating system for its default sans-serif font,
	// so we ship no font file. If that fails we fall back to the bitmap font.
	fontLoaded = font.load(OF_TTF_SANS, 12, true, true);
	if (!fontLoaded) ofLogWarning("ResultOverlay") << "system font not found, using bitmap font";
}

void ResultOverlay::setViewport(const ofRectangle& area) {
	viewport = area;
}

void ResultOverlay::fitImage(float imageW, float imageH) {
	if (imageW <= 0 || imageH <= 0) { imageRect = viewport; scale = 1.0f; return; }
	// Largest scale that keeps the whole picture inside the viewport.
	scale = std::min(viewport.width / imageW, viewport.height / imageH);
	const float w = imageW * scale, h = imageH * scale;
	imageRect.set(viewport.x + (viewport.width - w) * 0.5f,
	              viewport.y + (viewport.height - h) * 0.5f, w, h);
}

bool ResultOverlay::screenToImage(int sx, int sy, int& ix, int& iy) const {
	if (!imageRect.inside(float(sx), float(sy)) || scale <= 0.0f) return false;
	ix = int((sx - imageRect.x) / scale);
	iy = int((sy - imageRect.y) / scale);
	return true;
}

ofColor ResultOverlay::colourForClass(int classIndex) {
	static const ofColor palette[] = {
		ofColor(46, 204, 113), ofColor(52, 152, 219), ofColor(241, 196, 15),
		ofColor(231, 76, 60),  ofColor(155, 89, 182), ofColor(26, 188, 156),
		ofColor(230, 126, 34), ofColor(236, 64, 122)
	};
	if (classIndex < 0) return ofColor(46, 204, 113);      // unclassified: green
	return palette[classIndex % 8];
}

void ResultOverlay::draw(const ofTexture& frame, const ofTexture& mask, const DetectionResult& r,
                          ViewMode view, const std::vector<ObjectClass>& classes, const HudInfo& hud) {
	ofSetColor(30);
	ofDrawRectangle(viewport);

	if (!frame.isAllocated()) {
		ofSetColor(200);
		text("Drop an image here, press O to open one, or C for the camera.",
			viewport.x + 30, viewport.y + 40);
		if (hud.showHelp) drawHelp();
		return;
	}

	fitImage(frame.getWidth(), frame.getHeight());

	ofSetColor(255);
	if (view == ViewMode::Mask && mask.isAllocated()) mask.draw(imageRect);
	else                                               frame.draw(imageRect);

	if (view != ViewMode::Original) drawObjects(r);
	drawHud(r, classes, hud);
	if (hud.showHelp) drawHelp();
}

void ResultOverlay::drawObjects(const DetectionResult& r) {
	ofPushMatrix();
	ofTranslate(imageRect.x, imageRect.y);
	ofScale(scale, scale);                            // draw in picture pixels

	ofNoFill();
	int number = 1;
	for (const auto& o : r.objects) {
		const ofColor c = colourForClass(o.classIndex);

		ofPolyline outline;
		for (const auto& p : o.contour) outline.addVertex(float(p.x), float(p.y));
		outline.close();
		ofSetColor(c);
		ofSetLineWidth(2.0f);
		outline.draw();

		ofSetColor(c, 120);
		ofDrawRectangle(float(o.bbox.x), float(o.bbox.y), float(o.bbox.width), float(o.bbox.height));

		// Label: "3 orange #12" (number, class, track id when in video)
		std::string label = ofToString(number++);
		if (!o.label.empty()) label += " " + o.label;
		if (o.trackId >= 0)   label += " #" + ofToString(o.trackId);

		ofPushMatrix();
		ofTranslate(float(o.bbox.x), float(o.bbox.y) - 6.0f / scale);
		ofScale(1.0f / scale, 1.0f / scale);           // keep text readable at any zoom
		ofFill();
		ofSetColor(0, 160);
		ofDrawRectangle(-2, -14, textWidth(label) + 6, 18);
		ofSetColor(255);
		text(label, 1, 0);
		ofNoFill();
		ofPopMatrix();
	}
	ofFill();
	ofSetLineWidth(1.0f);
	ofPopMatrix();
}

void ResultOverlay::drawHud(const DetectionResult& r, const std::vector<ObjectClass>& classes,
                             const HudInfo& hud) {
	const float x = viewport.x + 12, lineH = 18;
	float y = viewport.y + 24;

	auto line = [&](const std::string& s, ofColor c = ofColor(255)) {
		ofSetColor(0, 150);
		ofDrawRectangle(x - 6, y - 14, textWidth(s) + 12, lineH);
		ofSetColor(c);
		text(s, x, y);
		y += lineH;
	};

	line("Objects: " + ofToString(r.totalCount), ofColor(255, 230, 90));
	if (hud.isVideo) line("Distinct so far: " + ofToString(r.totalTracked), ofColor(255, 230, 90));

	for (const auto& kv : r.countsByLabel) {
		if (kv.first == "object") continue;        // plain count already shown
		int idx = -1;
		for (size_t i = 0; i < classes.size(); ++i) if (classes[i].label == kv.first) idx = int(i);
		if (idx < 0 && kv.first.rfind("group ", 0) == 0) idx = ofToInt(kv.first.substr(6)) - 1;
		line("  " + kv.first + ": " + ofToString(kv.second), colourForClass(idx));
	}

	double mn = 0, mx = 0, mean = 0;
	ObjectDetector::sizeStats(r, mn, mx, mean);
	if (r.totalCount > 0) line("Area px  min " + ofToString(int(mn)) + "  max " +
		ofToString(int(mx)) + "  avg " + ofToString(int(mean)));

	line(hud.sourceName + "  |  " + hud.modeName + "  |  " + ofToString(r.processingMs, 1) + " ms  |  "
		+ ofToString(ofGetFrameRate(), 0) + " fps", ofColor(200));

	if (hud.teachMode)         line("TEACH MODE: click an object, type its name, press Enter", ofColor(120, 220, 255));
	if (!hud.prompt.empty())   line(hud.prompt, ofColor(120, 220, 255));
	if (!hud.message.empty()) line(hud.message, ofColor(180, 255, 180));
	line("H for help", ofColor(160));
}

void ResultOverlay::drawHelp() {
	const std::vector<std::string> lines = {
		"Keys",
		"O  open image        C  camera        V  open video",
		"1-5  mode: Otsu / manual / adaptive / colour pick / background removal",
		"Left click (colour pick mode)  choose the colour to select",
		"T  teach mode on/off   then click an object and type a name",
		"G  grayscale on/off    I  invert       X  split touching on/off",
		"Tab  view: original / mask / overlay",
		"R  reset video counter   S  screenshot   E  export CSV",
		"Space  pause video       H  this help   Esc  cancel typing",
		"Drag and drop an image or video onto the window to open it."
	};

	// Changed the box to follow its longest line, so adding a mode can never push text
	// past the edge. Clamped to the viewport in case the window is narrow.
	// Added as part of the 5th mode/strategy addition work
	float textW = 0.0f;
	for (const auto& l : lines) textW = std::max(textW, textWidth(l));
	const float w = std::min(textW + 24.0f, viewport.width - 32.0f);
	const float h = 18.0f * lines.size() + 20.0f;
	const float x = viewport.getRight() - w - 16, y = viewport.y + 16;

	ofSetColor(0, 200);
	ofDrawRectangle(x, y, w, h);
	ofSetColor(255);
	float ty = y + 22;
	for (const auto& l : lines) { text(l, x + 12, ty); ty += 18; }
}

void ResultOverlay::text(const std::string& s, float x, float y) {
	if (fontLoaded) font.drawString(s, x, y);
	else            ofDrawBitmapString(s, x, y);
}

float ResultOverlay::textWidth(const std::string& s) {
	if (fontLoaded) return font.stringWidth(s);
	return 8.0f * s.size();
}
