// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    CvBridge.cpp
//  Author:  Ronald K. Sebuhinja
//  Purpose: ofPixels <-> cv::Mat conversions.
// -----------------------------------------------------------------------------
#include "CvBridge.h"

#include <opencv2/imgproc.hpp>

namespace CvBridge {

cv::Mat toMat(const ofPixels& pixels) {
	if (!pixels.isAllocated()) return cv::Mat();

	const int w = int(pixels.getWidth());
	const int h = int(pixels.getHeight());
	const int channels = int(pixels.getNumChannels());

	// cv::Mat wants a non-const pointer even when we only read from it.
	unsigned char* data = const_cast<unsigned char*>(pixels.getData());

	if (channels == 3) return cv::Mat(h, w, CV_8UC3, data, pixels.getBytesStride());
	if (channels == 1) return cv::Mat(h, w, CV_8UC1, data, pixels.getBytesStride());
	if (channels == 4) {
		cv::Mat rgba(h, w, CV_8UC4, data, pixels.getBytesStride());
		cv::Mat rgb;
		cv::cvtColor(rgba, rgb, cv::COLOR_RGBA2RGB);   // this one is a copy
		return rgb;
	}
	return cv::Mat();
}

// The texture always receives RGB so we never depend on GL_LUMINANCE support.
static cv::Mat asContinuousRgb(const cv::Mat& mat) {
	cv::Mat rgb;
	if (mat.channels() == 1) cv::cvtColor(mat, rgb, cv::COLOR_GRAY2RGB);
	else if (mat.isContinuous()) rgb = mat;
	else rgb = mat.clone();
	return rgb;
}

void toTexture(const cv::Mat& mat, ofTexture& texture) {
	if (mat.empty()) return;
	const cv::Mat rgb = asContinuousRgb(mat);
	if (!texture.isAllocated() || int(texture.getWidth()) != rgb.cols || int(texture.getHeight()) != rgb.rows) {
		texture.allocate(rgb.cols, rgb.rows, GL_RGB);
	}
	texture.loadData(rgb.data, rgb.cols, rgb.rows, GL_RGB);
}

void toImage(const cv::Mat& mat, ofImage& image) {
	if (mat.empty()) return;
	const cv::Mat rgb = asContinuousRgb(mat);
	image.setFromPixels(rgb.data, rgb.cols, rgb.rows, OF_IMAGE_COLOR);
}

} // namespace CvBridge

