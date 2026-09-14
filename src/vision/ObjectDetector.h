// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ObjectDetector.h
//  Author:  Ronald Sebuhinja
//  Purpose: The one door into the vision pipeline. The app hands in a frame
//           and settings and gets back a DetectionResult. Internally it runs
//           Segmenter -> ContourAnalyser -> ObjectClassifier -> ObjectTracker.
//           No openFrameworks code in here: this compiles with plain OpenCV.
//           FROZEN public interface after Day 1.
// -----------------------------------------------------------------------------
#pragma once

#include <opencv2/core.hpp>
#include <map>
#include <string>
#include <vector>

#include "ContourAnalyser.h"
#include "DetectedObject.h"
#include "DetectionSettings.h"
#include "ObjectClassifier.h"
#include "ObjectTracker.h"
#include "Segmenter.h"

struct DetectionResult {
	cv::Mat                     mask;          // processed-size binary mask (for the "mask" view)
	std::vector<DetectedObject> objects;       // coordinates in original frame pixels
	std::map<std::string, int>  countsByLabel; // e.g. {"orange": 7, "pineapple": 2}
	int                         totalCount     = 0;
	int                         totalTracked   = 0;   // video only: distinct objects seen so far
	double                      processingMs   = 0.0;
	float                       scale          = 1.0f; // original / processed
	cv::Size                    processedSize;
	std::string                 modeName;
};

class ObjectDetector {
public:
	// frame: 8-bit, 1 or 3 channels (RGB). isVideo enables tracking.
	void process(const cv::Mat& frame, const DetectionSettings& s, bool isVideo, DetectionResult& out);

	// Sample the colour under a point of the ORIGINAL frame into the settings.
	void pickSeedColour(const cv::Mat& frame, cv::Point p, DetectionSettings& s) const;

	// Which object (if any) contains this point of the original frame. nullptr if none.
	static const DetectedObject* objectAt(const DetectionResult& r, cv::Point p);

	ObjectClassifier& getClassifier() { return classifier; }
	ObjectTracker&    getTracker()    { return tracker; }

	// Size statistics of the current result, in original-frame pixels.
	static void sizeStats(const DetectionResult& r, double& minArea, double& maxArea, double& meanArea);

private:
	Segmenter        segmenter;     // composition: the detector owns its stages
	ContourAnalyser  analyser;
	ObjectClassifier classifier;
	ObjectTracker    tracker;

	cv::Mat small;                  // scratch: the shrunken frame
};

