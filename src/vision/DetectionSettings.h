// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    DetectionSettings.h
//  Author:  Ronald K. Sebuhinja
//  Purpose: Every tunable the user can change, in one plain struct.
//           The GUI writes into it, the vision pipeline reads from it.
//           FROZEN after Day 1 - tell your team mate before changing it.
// -----------------------------------------------------------------------------
#pragma once

// How we turn a picture into a black-and-white mask of "object vs background".
enum class SegmentationMode {
	AutoOtsu = 0,     // Otsu picks the threshold automatically
	ManualThreshold,  // user picks the threshold with a slider
	Adaptive,         // threshold adapts to local brightness (uneven lighting)
	ColourPick,        // user clicks a colour, tolerance slider decides how similar is "same"
	BackgroundRemove   // anything unlike the background colour is an object (fifth approach)
};

struct DetectionSettings {
	// --- segmentation ---
	SegmentationMode mode   = SegmentationMode::AutoOtsu;
	bool  forceGrayscale    = false;  // process a colour picture as black and white
	int   threshold         = 128;    // ManualThreshold only (0..255)
	bool  invert            = false;  // true when objects are brighter than the background
	bool  autoInvert        = true;   // AutoOtsu only: assume objects are the smaller area
	int   minContrast       = 30;     // AutoOtsu only: below this gap the scene counts as empty
	int   adaptiveBlockSize = 51;     // Adaptive only, odd number >= 3
	int   adaptiveOffset    = 5;      // Adaptive only

	// --- colour pick (the "magic wand") ---
	bool  hasSeedColour     = false;
	float seedHue           = 0.0f;   // OpenCV hue range 0..180
	float seedSaturation    = 0.0f;   // 0..255
	float seedValue         = 0.0f;   // 0..255
	float tolerance         = 35.0f;  // 0..100, modes 3 and 4: how similar counts as "same"

	// --- clean-up of the mask ---
	int   blurSize          = 5;      // odd number, 1 = no blur
	int   openIterations    = 1;      // removes speckles
	int   closeIterations   = 1;      // closes small gaps
	bool  fillHoles         = true;   // fills holes inside objects
	bool  splitTouching     = false;  // separate objects that touch (watershed)
	float splitStrength     = 0.5f;   // 0.1..0.9, higher = more aggressive splitting

	// --- which blobs count as objects ---
	float minArea           = 300.0f; // pixels, in the processed frame
	float maxArea           = 0.0f;   // 0 = no upper limit

	// --- classification ---
	bool  useColourFeatures = true;
	bool  useShapeFeatures  = true;
	float maxMatchDistance  = 0.45f;  // bigger = more forgiving when matching taught classes
	bool  autoGroup         = false;  // k-means grouping when nothing has been taught
	int   groupCount        = 2;

	// --- video ---
	float trackMaxDistance  = 60.0f;  // pixels an object may move between frames
	int   trackMaxMissed    = 10;     // frames a track survives without a match
	int   trackMinFrames    = 3;      // frames before a track counts as "seen"

	// --- performance ---
	int   processWidth      = 640;    // frames are shrunk to this width before processing
};

// Lets the app skip re-processing a still image when nothing changed.
inline bool operator==(const DetectionSettings& a, const DetectionSettings& b) {
	return a.mode == b.mode && a.forceGrayscale == b.forceGrayscale
		&& a.threshold == b.threshold && a.invert == b.invert && a.autoInvert == b.autoInvert
		&& a.minContrast == b.minContrast
		&& a.adaptiveBlockSize == b.adaptiveBlockSize && a.adaptiveOffset == b.adaptiveOffset
		&& a.hasSeedColour == b.hasSeedColour && a.seedHue == b.seedHue
		&& a.seedSaturation == b.seedSaturation && a.seedValue == b.seedValue
		&& a.tolerance == b.tolerance && a.blurSize == b.blurSize
		&& a.openIterations == b.openIterations && a.closeIterations == b.closeIterations
		&& a.fillHoles == b.fillHoles && a.splitTouching == b.splitTouching
		&& a.splitStrength == b.splitStrength && a.minArea == b.minArea && a.maxArea == b.maxArea
		&& a.useColourFeatures == b.useColourFeatures && a.useShapeFeatures == b.useShapeFeatures
		&& a.maxMatchDistance == b.maxMatchDistance && a.autoGroup == b.autoGroup
		&& a.groupCount == b.groupCount && a.trackMaxDistance == b.trackMaxDistance
		&& a.trackMaxMissed == b.trackMaxMissed && a.trackMinFrames == b.trackMinFrames
		&& a.processWidth == b.processWidth;
}

inline bool operator!=(const DetectionSettings& a, const DetectionSettings& b) { return !(a == b); }

