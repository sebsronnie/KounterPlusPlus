// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    CvBridge.h
//  Author:  Ronald K. Sebuhinja
//  Purpose: The only place where openFrameworks pixels and OpenCV matrices
//           meet. Everything else talks either oF or OpenCV, never both.
// -----------------------------------------------------------------------------
#pragma once

#include "ofMain.h"
#include <opencv2/core.hpp>

namespace CvBridge {

	// Wraps the pixel memory in a cv::Mat WITHOUT copying. The Mat is only
	// valid while the ofPixels object is alive and unchanged. Handles
	// grayscale (1 channel) and RGB (3 channels). RGBA is converted (copied).
	cv::Mat toMat(const ofPixels& pixels);

	// Uploads a 1- or 3-channel 8-bit Mat into a texture for drawing.
	void toTexture(const cv::Mat& mat, ofTexture& texture);

	// Copies a 1- or 3-channel 8-bit Mat into an ofImage (used for saving).
	void toImage(const cv::Mat& mat, ofImage& image);
}

