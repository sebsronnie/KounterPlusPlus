// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    DetectedObject.h
//  Author:  Ronald K. Sebuhinja
//  Purpose: One found object and everything we measured about it.
//           Plain value type: copying it is safe and cheap enough.
//           FROZEN after Day 1 - tell your team mate before changing it.
// -----------------------------------------------------------------------------
#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

struct DetectedObject {
	// Geometry, in the coordinates of the ORIGINAL frame (not the shrunken one).
	std::vector<cv::Point> contour;
	cv::Rect     bbox;
	cv::Point2f  centroid;
	double       area        = 0.0;   // pixels
	double       perimeter   = 0.0;   // pixels

	// Shape features, all roughly in the range 0..1.
	double       circularity = 0.0;   // 1.0 = perfect circle
	double       aspectRatio = 1.0;   // short side / long side of the bounding box
	double       solidity    = 0.0;   // area / convex hull area (1.0 = no dents)

	// Colour features, OpenCV HSV ranges: hue 0..180, saturation 0..255, value 0..255.
	double       meanHue        = 0.0;
	double       meanSaturation = 0.0;
	double       meanValue      = 0.0;

	// Classification and tracking results.
	std::string  label;               // empty = not classified
	int          classIndex = -1;     // index into the classifier's class list, -1 = none
	int          trackId    = -1;     // stable id across video frames, -1 = still image
};

