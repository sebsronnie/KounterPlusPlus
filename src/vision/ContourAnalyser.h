// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ContourAnalyser.h
//  Author:  Ronald Sebuhinja
//  Purpose: Finds the blobs in a mask and measures each one
//           (size, shape and average colour).
// -----------------------------------------------------------------------------
#pragma once

#include <opencv2/core.hpp>
#include <vector>

#include "DetectedObject.h"
#include "DetectionSettings.h"

class ContourAnalyser {
public:
	// mask: 8-bit single channel, 255 = object.
	// rgb:  the frame the mask was made from (1 or 3 channels), same size.
	// objects: cleared and refilled. Coordinates are in mask/rgb pixels.
	void analyse(const cv::Mat& mask, const cv::Mat& rgb,
				 const DetectionSettings& s, std::vector<DetectedObject>& objects);

private:
	void measure(DetectedObject& obj, const cv::Mat& rgb, const cv::Mat& hsv);

	// Scratch buffers reused across frames.
	cv::Mat maskCopy, hsv, objectMask;
	std::vector<std::vector<cv::Point>> contours;
};

