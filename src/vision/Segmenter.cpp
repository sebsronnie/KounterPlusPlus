// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    Segmenter.cpp
//  Author:  Ronald K. Sebuhinja
//  Purpose: Implementation of the segmentation strategies and mask clean-up.
// -----------------------------------------------------------------------------
#include "Segmenter.h"

#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------- strategies

void OtsuStrategy::apply(const cv::Mat& gray, const cv::Mat&, const DetectionSettings& s, cv::Mat& mask) const {
	cv::threshold(gray, mask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

	// Otsu ALWAYS finds a threshold, even on an empty table, where it would
	// happily split the noise into dozens of "objects". If the bright and the
	// dark halves are not clearly different, we call the scene empty.
	cv::Mat notMask;
	cv::bitwise_not(mask, notMask);
	const double gap = cv::mean(gray, mask)[0] - cv::mean(gray, notMask)[0];
	if (gap < double(s.minContrast)) { mask.setTo(0); return; }

	// Objects are usually the smaller part of the picture. If more than half
	// of the mask is white, we have most likely selected the background.
	bool invert = s.invert;
	if (s.autoInvert) {
		const double whiteFraction = cv::countNonZero(mask) / double(mask.total());
		invert = whiteFraction > 0.5;
	}
	if (invert) cv::bitwise_not(mask, mask);
}

void ManualThresholdStrategy::apply(const cv::Mat& gray, const cv::Mat&, const DetectionSettings& s, cv::Mat& mask) const {
	const int type = s.invert ? cv::THRESH_BINARY : cv::THRESH_BINARY_INV;
	cv::threshold(gray, mask, s.threshold, 255, type);
}

void AdaptiveStrategy::apply(const cv::Mat& gray, const cv::Mat&, const DetectionSettings& s, cv::Mat& mask) const {
	int block = std::max(3, s.adaptiveBlockSize);
	if (block % 2 == 0) block += 1;                       // OpenCV needs an odd block size
	const int type = s.invert ? cv::THRESH_BINARY : cv::THRESH_BINARY_INV;
	cv::adaptiveThreshold(gray, mask, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, type, block, s.adaptiveOffset);
}

void ColourPickStrategy::apply(const cv::Mat& gray, const cv::Mat& hsv, const DetectionSettings& s, cv::Mat& mask) const {
	if (!s.hasSeedColour) {                               // nothing picked yet: empty mask
		mask = cv::Mat::zeros(gray.size(), CV_8UC1);
		return;
	}
	const float maxDist = std::max(0.01f, s.tolerance / 100.0f);

	if (hsv.empty()) {
		// Grayscale input: "colour" is just brightness.
		cv::Mat diff;
		cv::absdiff(gray, cv::Scalar(s.seedValue), diff);
		cv::threshold(diff, mask, maxDist * 255.0, 255, cv::THRESH_BINARY_INV);
		return;
	}

	// Distance in HSV space. Hue is circular (0 and 180 are the same colour),
	// so we take the shorter way around the circle.
	std::vector<cv::Mat> ch;
	cv::split(hsv, ch);
	cv::Mat h, sat, val;
	ch[0].convertTo(h,   CV_32F);
	ch[1].convertTo(sat, CV_32F, 1.0 / 255.0);
	ch[2].convertTo(val, CV_32F, 1.0 / 255.0);

	cv::Mat dh;
	cv::absdiff(h, cv::Scalar(s.seedHue), dh);
	dh = cv::min(dh, 180.0f - dh);
	dh /= 90.0f;                                          // now 0..1

	cv::Mat ds, dv;
	cv::absdiff(sat, cv::Scalar(s.seedSaturation / 255.0f), ds);
	cv::absdiff(val, cv::Scalar(s.seedValue / 255.0f), dv);

	// Hue matters most, then saturation, then brightness (lighting changes it).
	// Grey-ish seed colours have no reliable hue, so hue weight fades out.
	const float satWeightOfSeed = std::min(1.0f, s.seedSaturation / 80.0f);
	const float wH = 1.0f * satWeightOfSeed, wS = 0.6f, wV = 0.4f;

	cv::Mat dist;
	cv::sqrt(wH * dh.mul(dh) + wS * ds.mul(ds) + wV * dv.mul(dv), dist);

	cv::Mat within = dist < maxDist;                      // 255 where similar
	within.copyTo(mask);
}

// ----------------------------------------------------------------- Segmenter

Segmenter::Segmenter() {
	strategies.push_back(std::make_unique<OtsuStrategy>());
	strategies.push_back(std::make_unique<ManualThresholdStrategy>());
	strategies.push_back(std::make_unique<AdaptiveStrategy>());
	strategies.push_back(std::make_unique<ColourPickStrategy>());
	strategies.push_back(std::make_unique<BackgroundRemoveStrategy>());
}

const SegmentationStrategy& Segmenter::strategyFor(SegmentationMode m) const {
	for (const auto& st : strategies) {
		if (st->mode() == m) return *st;
	}
	return *strategies.front();                           // safe default
}

std::string Segmenter::strategyName(SegmentationMode m) const {
	return strategyFor(m).name();
}

void Segmenter::segment(const cv::Mat& rgb, const DetectionSettings& s, cv::Mat& mask) {
	const bool isColour = rgb.channels() == 3 && !s.forceGrayscale;

	if (rgb.channels() == 3) cv::cvtColor(rgb, gray, cv::COLOR_RGB2GRAY);
	else                     gray = rgb;

	if (isColour) cv::cvtColor(rgb, hsv, cv::COLOR_RGB2HSV);
	else          hsv.release();                          // tells strategies "no colour"

	int k = std::max(1, s.blurSize);
	if (k % 2 == 0) k += 1;
	if (k > 1) cv::GaussianBlur(gray, blurred, cv::Size(k, k), 0);
	else       blurred = gray;

	strategyFor(s.mode).apply(blurred, hsv, s, mask);     // polymorphic call
	cleanUp(mask, s);
	if (s.splitTouching) splitTouching(mask, rgb, s);
}

void Segmenter::cleanUp(cv::Mat& mask, const DetectionSettings& s) const {
	const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	if (s.openIterations  > 0) cv::morphologyEx(mask, mask, cv::MORPH_OPEN,  kernel, cv::Point(-1, -1), s.openIterations);
	if (s.closeIterations > 0) cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), s.closeIterations);
	if (s.fillHoles) fillHoles(mask);
}

// Flood-fill the background from a 1-pixel border; whatever is still black
// afterwards is a hole inside an object, so we paint it white.
void Segmenter::fillHoles(cv::Mat& mask) const {
	cv::Mat padded;
	cv::copyMakeBorder(mask, padded, 1, 1, 1, 1, cv::BORDER_CONSTANT, cv::Scalar(0));
	cv::Mat flood = padded.clone();
	cv::floodFill(flood, cv::Point(0, 0), cv::Scalar(255));
	cv::Mat holes;
	cv::bitwise_not(flood, holes);
	padded |= holes;
	padded(cv::Rect(1, 1, mask.cols, mask.rows)).copyTo(mask);
}

// Classic distance-transform + watershed. Each object's "core" (far from any
// edge) becomes a marker, and watershed grows the markers until they meet.
void Segmenter::splitTouching(cv::Mat& mask, const cv::Mat& rgb, const DetectionSettings& s) const {
	cv::Mat dist;
	cv::distanceTransform(mask, dist, cv::DIST_L2, 5);
	double maxVal = 0.0;
	cv::minMaxLoc(dist, nullptr, &maxVal);
	if (maxVal < 2.0) return;                             // nothing big enough to split

	const float strength = std::min(0.9f, std::max(0.1f, s.splitStrength));
	cv::Mat sureFg;
	cv::threshold(dist, sureFg, strength * maxVal, 255, cv::THRESH_BINARY);
	sureFg.convertTo(sureFg, CV_8U);

	cv::Mat markers;
	const int n = cv::connectedComponents(sureFg, markers, 8, CV_32S);
	if (n <= 2) return;                                   // only one core found: nothing to split

	// markers: 0 = unknown region, 1 = background, 2..n = object cores
	markers += 1;
	cv::Mat unknown;
	cv::subtract(mask, sureFg, unknown);
	markers.setTo(0, unknown);

	cv::Mat image3;
	if (rgb.channels() == 3) image3 = rgb;
	else                     cv::cvtColor(rgb, image3, cv::COLOR_GRAY2RGB);
	cv::watershed(image3, markers);

	// Watershed marks the dividing lines with -1. Those lines are one pixel
	// wide and findContours joins pixels diagonally, so we thicken the lines
	// to three pixels before cutting them out of the mask.
	cv::Mat boundary = (markers == -1);
	cv::dilate(boundary, boundary, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
	mask.setTo(0, boundary);
}


namespace {

// Median colour of a band around the edge of the frame. Median rather than mean
// so that an object touching the border cannot drag the estimate.
void borderColour(const cv::Mat& img, int band, std::vector<float>& out) {
	const int w = img.cols, h = img.rows, ch = img.channels();
	band = std::max(1, std::min(band, std::min(w, h) / 4));
	std::vector<std::vector<unsigned char>> samples(ch);
	for (int y = 0; y < h; ++y) {
		const bool edgeRow = (y < band || y >= h - band);
		for (int x = 0; x < w; ++x) {
			if (!edgeRow && x >= band && x < w - band) continue;
			const unsigned char* p = img.ptr<unsigned char>(y) + x * ch;
			for (int c = 0; c < ch; ++c) samples[c].push_back(p[c]);
		}
	}
	out.assign(ch, 0.0f);
	for (int c = 0; c < ch; ++c) {
		auto& v = samples[c];
		if (v.empty()) continue;
		std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
		out[c] = float(v[v.size() / 2]);
	}
}

} // namespace

void BackgroundRemoveStrategy::apply(const cv::Mat& gray, const cv::Mat& hsv,
									 const DetectionSettings& s, cv::Mat& mask) const {
	const float maxDist = std::max(0.01f, s.tolerance / 100.0f);
	const int   band    = std::max(2, gray.cols / 64);

	if (hsv.empty()) {
		// Grayscale input: "unlike the background" can only mean "different brightness".
		std::vector<float> bg;
		borderColour(gray, band, bg);
		cv::Mat diff;
		cv::absdiff(gray, cv::Scalar(bg[0]), diff);
		cv::threshold(diff, mask, maxDist * 255.0, 255, cv::THRESH_BINARY);
		return;
	}

	std::vector<float> bg;                       // hue, saturation, value of the border
	borderColour(hsv, band, bg);

	std::vector<cv::Mat> ch;
	cv::split(hsv, ch);
	cv::Mat h, sat, val;
	ch[0].convertTo(h,   CV_32F);
	ch[1].convertTo(sat, CV_32F, 1.0 / 255.0);
	ch[2].convertTo(val, CV_32F, 1.0 / 255.0);

	cv::Mat dh;
	cv::absdiff(h, cv::Scalar(bg[0]), dh);
	dh = cv::min(dh, 180.0f - dh);               // hue is circular
	dh /= 90.0f;

	cv::Mat ds, dv;
	cv::absdiff(sat, cv::Scalar(bg[1] / 255.0f), ds);
	cv::absdiff(val, cv::Scalar(bg[2] / 255.0f), dv);

	// A white, grey or black background has no meaningful hue of its own, so hue
	// only counts in proportion to how colourful the background actually is.
	// Saturation carries most of the weight: on a plain backdrop, "colourful at
	// all" is the most reliable sign that a pixel belongs to an object.
	const float bgSat = std::min(1.0f, bg[1] / 80.0f);
	const float wH = 1.0f * bgSat, wS = 1.0f, wV = 0.8f;

	cv::Mat dist;
	cv::sqrt(wH * dh.mul(dh) + wS * ds.mul(ds) + wV * dv.mul(dv), dist);

	cv::Mat away = dist > maxDist;               // FAR from the background = object
	away.copyTo(mask);
}
