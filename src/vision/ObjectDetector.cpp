// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ObjectDetector.cpp
//  Author:  Ronald Sebuhinja
//  Purpose: Runs the pipeline stages in order and scales results back to
//           original frame coordinates.
// -----------------------------------------------------------------------------
#include "ObjectDetector.h"

#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <chrono>

void ObjectDetector::process(const cv::Mat& frame, const DetectionSettings& s, bool isVideo, DetectionResult& out) {
	const auto t0 = std::chrono::steady_clock::now();

	// 1. Shrink for speed. Everything downstream works on "small".
	float scale = 1.0f;
	if (s.processWidth > 0 && frame.cols > s.processWidth) {
		scale = float(frame.cols) / float(s.processWidth);
		const int h = std::max(1, int(frame.rows / scale + 0.5f));
		cv::resize(frame, small, cv::Size(s.processWidth, h), 0, 0, cv::INTER_AREA);
	} else {
		small = frame;
	}

	// 2. Pixels -> mask -> measured objects -> labels (-> track ids).
	segmenter.segment(small, s, out.mask);
	analyser.analyse(out.mask, small, s, out.objects);
	classifier.classify(out.objects, s);

	// 3. Back to original coordinates so the UI never needs to know about "small".
	if (scale != 1.0f) {
		for (auto& o : out.objects) {
			for (auto& p : o.contour) { p.x = int(p.x * scale + 0.5f); p.y = int(p.y * scale + 0.5f); }
			o.bbox      = cv::Rect(int(o.bbox.x * scale), int(o.bbox.y * scale),
								   int(o.bbox.width * scale), int(o.bbox.height * scale));
			o.centroid *= scale;
			o.area      *= double(scale) * double(scale);
			o.perimeter *= scale;
		}
	}

	if (isVideo) {
		tracker.update(out.objects, s);
		out.totalTracked = tracker.totalCounted();
	} else {
		out.totalTracked = 0;
	}

	// 4. Summary numbers for the HUD.
	out.countsByLabel.clear();
	for (const auto& o : out.objects) {
		out.countsByLabel[o.label.empty() ? "object" : o.label]++;
	}
	out.totalCount    = int(out.objects.size());
	out.scale         = scale;
	out.processedSize = small.size();
	out.modeName      = segmenter.strategyName(s.mode);

	const auto t1 = std::chrono::steady_clock::now();
	out.processingMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
}

void ObjectDetector::pickSeedColour(const cv::Mat& frame, cv::Point p, DetectionSettings& s) const {
	if (frame.empty()) return;
	p.x = std::max(0, std::min(frame.cols - 1, p.x));
	p.y = std::max(0, std::min(frame.rows - 1, p.y));

	// Average a 5x5 patch so a single noisy pixel does not decide the colour.
	const cv::Rect patch(std::max(0, p.x - 2), std::max(0, p.y - 2), 5, 5);
	const cv::Rect safe = patch & cv::Rect(0, 0, frame.cols, frame.rows);

	if (frame.channels() == 3) {
		cv::Mat hsv;
		cv::cvtColor(frame(safe), hsv, cv::COLOR_RGB2HSV);
		const cv::Scalar m = cv::mean(hsv);
		s.seedHue = float(m[0]); s.seedSaturation = float(m[1]); s.seedValue = float(m[2]);
	} else {
		const cv::Scalar m = cv::mean(frame(safe));
		s.seedHue = 0.0f; s.seedSaturation = 0.0f; s.seedValue = float(m[0]);
	}
	s.hasSeedColour = true;
}

const DetectedObject* ObjectDetector::objectAt(const DetectionResult& r, cv::Point p) {
	for (const auto& o : r.objects) {
		if (!o.bbox.contains(p)) continue;                                  // cheap test first
		if (cv::pointPolygonTest(o.contour, cv::Point2f(float(p.x), float(p.y)), false) >= 0) return &o;
	}
	return nullptr;
}

void ObjectDetector::sizeStats(const DetectionResult& r, double& minArea, double& maxArea, double& meanArea) {
	minArea = maxArea = meanArea = 0.0;
	if (r.objects.empty()) return;
	minArea = maxArea = r.objects.front().area;
	double sum = 0.0;
	for (const auto& o : r.objects) {
		minArea = std::min(minArea, o.area);
		maxArea = std::max(maxArea, o.area);
		sum += o.area;
	}
	meanArea = sum / double(r.objects.size());
}

