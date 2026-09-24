// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ControlPanel.cpp
//  Author:  Kelvin Maritim
//  Purpose: Builds the ofxGui panel and maps it to DetectionSettings.
// ---------------------------------------------------------------------------
#include "ControlPanel.h"

ControlPanel::ControlPanel() {
	loadImageButton.addListener(this, &ControlPanel::loadImagePressed);
	cameraButton.addListener(this, &ControlPanel::cameraPressed);
	videoButton.addListener(this, &ControlPanel::videoPressed);
	clearClassesButton.addListener(this, &ControlPanel::clearClassesPressed);
	resetCounterButton.addListener(this, &ControlPanel::resetCounterPressed);
	screenshotButton.addListener(this, &ControlPanel::screenshotPressed);
	csvButton.addListener(this, &ControlPanel::csvPressed);
}

ControlPanel::~ControlPanel() {
	// Listeners hold a pointer to this object: remove them before we go away.
	loadImageButton.removeListener(this, &ControlPanel::loadImagePressed);
	cameraButton.removeListener(this, &ControlPanel::cameraPressed);
	videoButton.removeListener(this, &ControlPanel::videoPressed);
	clearClassesButton.removeListener(this, &ControlPanel::clearClassesPressed);
	resetCounterButton.removeListener(this, &ControlPanel::resetCounterPressed);
	screenshotButton.removeListener(this, &ControlPanel::screenshotPressed);
	csvButton.removeListener(this, &ControlPanel::csvPressed);
}

void ControlPanel::setup(float x, float y) {
	// A fresh struct supplies the defaults, so the GUI and the pipeline can
	// never disagree about what "default" means.
	const DetectionSettings d;

	segmentation.setName("Segmentation");
	segmentation.add(mode.set("Mode 0-3 (keys 1-4)", int(d.mode), 0, 3));
	segmentation.add(forceGrayscale.set("Force grayscale", d.forceGrayscale));
	segmentation.add(autoInvert.set("Auto invert (Otsu)", d.autoInvert));
	segmentation.add(invert.set("Invert", d.invert));
	segmentation.add(minContrast.set("Min contrast (Otsu)", d.minContrast, 0, 100));
	segmentation.add(threshold.set("Threshold", d.threshold, 0, 255));
	segmentation.add(adaptiveBlockSize.set("Adaptive block", d.adaptiveBlockSize, 3, 151));
	segmentation.add(adaptiveOffset.set("Adaptive offset", d.adaptiveOffset, -30, 30));
	segmentation.add(tolerance.set("Colour tolerance", d.tolerance, 0.0f, 100.0f));
	segmentation.add(mode.set("Mode 0-4 (keys 1-5)", int(d.mode), 0, 4));
	segmentation.add(tolerance.set("Tolerance (modes 3,4)", d.tolerance, 0.0f, 100.0f));

	cleanup.setName("Clean-up");
	cleanup.add(blurSize.set("Blur", d.blurSize, 1, 21));
	cleanup.add(openIterations.set("Remove speckles", d.openIterations, 0, 5));
	cleanup.add(closeIterations.set("Close gaps", d.closeIterations, 0, 5));
	cleanup.add(fillHoles.set("Fill holes", d.fillHoles));
	cleanup.add(splitTouching.set("Split touching", d.splitTouching));
	cleanup.add(splitStrength.set("Split strength", d.splitStrength, 0.1f, 0.9f));

	filtering.setName("Size filter");
	filtering.add(minArea.set("Min area px", d.minArea, 0.0f, 20000.0f));
	filtering.add(maxArea.set("Max area px (0=off)", d.maxArea, 0.0f, 200000.0f));

	classification.setName("Classification");
	classification.add(useColour.set("Use colour", d.useColourFeatures));
	classification.add(useShape.set("Use shape", d.useShapeFeatures));
	classification.add(maxMatchDistance.set("Match tolerance", d.maxMatchDistance, 0.05f, 1.5f));
	classification.add(autoGroup.set("Auto group", d.autoGroup));
	classification.add(groupCount.set("Groups", d.groupCount, 2, 6));

	video.setName("Video & speed");
	video.add(trackMaxDistance.set("Track distance", d.trackMaxDistance, 10.0f, 300.0f));
	video.add(trackMaxMissed.set("Track patience", d.trackMaxMissed, 1, 60));
	video.add(processWidth.set("Process width", d.processWidth, 160, 1920));

	ofxGuiSetDefaultWidth(240);                   // a little wider than the default 200
	gui.setup("Object Counter", "settings.xml", x, y);
	gui.add(loadImageButton.setup("Load image (O)"));
	gui.add(cameraButton.setup("Use camera (C)"));
	gui.add(videoButton.setup("Load video (V)"));
	gui.add(segmentation);
	gui.add(cleanup);
	gui.add(filtering);
	gui.add(classification);
	gui.add(video);
	gui.add(clearClassesButton.setup("Clear taught classes"));
	gui.add(resetCounterButton.setup("Reset video counter (R)"));
	gui.add(screenshotButton.setup("Save screenshot (S)"));
	gui.add(csvButton.setup("Export CSV (E)"));
}

void ControlPanel::draw() {
	gui.draw();
}

void ControlPanel::applyTo(DetectionSettings& s) const {
	s.mode              = SegmentationMode(mode.get());
	s.forceGrayscale    = forceGrayscale;
	s.autoInvert        = autoInvert;
	s.invert            = invert;
	s.minContrast       = minContrast;
	s.threshold         = threshold;
	s.adaptiveBlockSize = adaptiveBlockSize;
	s.adaptiveOffset    = adaptiveOffset;
	s.tolerance         = tolerance;

	s.blurSize          = blurSize;
	s.openIterations    = openIterations;
	s.closeIterations   = closeIterations;
	s.fillHoles         = fillHoles;
	s.splitTouching     = splitTouching;
	s.splitStrength     = splitStrength;

	s.minArea           = minArea;
	s.maxArea           = maxArea;

	s.useColourFeatures = useColour;
	s.useShapeFeatures  = useShape;
	s.maxMatchDistance  = maxMatchDistance;
	s.autoGroup         = autoGroup;
	s.groupCount        = groupCount;

	s.trackMaxDistance  = trackMaxDistance;
	s.trackMaxMissed    = trackMaxMissed;
	s.processWidth      = processWidth;
	// seed colour and hasSeedColour are owned by the settings struct itself
}

void ControlPanel::readFrom(const DetectionSettings& s) {
	mode               = int(s.mode);
	forceGrayscale     = s.forceGrayscale;
	autoInvert         = s.autoInvert;
	invert             = s.invert;
	minContrast        = s.minContrast;
	threshold          = s.threshold;
	adaptiveBlockSize  = s.adaptiveBlockSize;
	adaptiveOffset     = s.adaptiveOffset;
	tolerance          = s.tolerance;
	blurSize           = s.blurSize;
	openIterations     = s.openIterations;
	closeIterations    = s.closeIterations;
	fillHoles          = s.fillHoles;
	splitTouching      = s.splitTouching;
	splitStrength      = s.splitStrength;
	minArea            = s.minArea;
	maxArea            = s.maxArea;
	useColour          = s.useColourFeatures;
	useShape           = s.useShapeFeatures;
	maxMatchDistance   = s.maxMatchDistance;
	autoGroup          = s.autoGroup;
	groupCount         = s.groupCount;
	trackMaxDistance   = s.trackMaxDistance;
	trackMaxMissed     = s.trackMaxMissed;
	processWidth       = s.processWidth;
}

bool ControlPanel::isMouseOver(int x, int y) const {
	return gui.getShape().inside(float(x), float(y));
}
