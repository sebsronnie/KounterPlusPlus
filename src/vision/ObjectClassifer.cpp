// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ObjectClassifier.cpp
//  Author:  Ronald Sebuhinja
//  Purpose: Nearest-prototype classification and k-means grouping.
// -----------------------------------------------------------------------------
#include "ObjectClassifier.h"

#include <opencv2/core.hpp>
#include <algorithm>
#include <cmath>

std::vector<float> ObjectClassifier::featureVector(const DetectedObject& obj, const DetectionSettings& s) {
	std::vector<float> f;

	if (s.useColourFeatures) {
		// Hue as a point on a circle, so red (0) and red (180) are neighbours.
		const double angle = obj.meanHue * (CV_PI / 90.0);
		const float  sat   = float(obj.meanSaturation / 255.0);
		// Grey objects have no meaningful hue: scale the hue point by saturation.
		f.push_back(float(std::cos(angle)) * sat);
		f.push_back(float(std::sin(angle)) * sat);
		f.push_back(0.7f * sat);
		f.push_back(0.3f * float(obj.meanValue / 255.0));
	}
	if (s.useShapeFeatures) {
		// log10(area): 1000 px -> 3, 100000 px -> 5. Divide so it is ~0..1.
		f.push_back(0.6f * float(std::log10(std::max(1.0, obj.area)) / 6.0));
		f.push_back(1.0f * float(obj.circularity));
		f.push_back(0.6f * float(obj.aspectRatio));
		f.push_back(0.5f * float(obj.solidity));
	}
	return f;
}

float ObjectClassifier::distance(const std::vector<float>& a, const std::vector<float>& b) {
	if (a.size() != b.size()) return 1e9f;   // taught with different settings: never matches
	float sum = 0.0f;
	for (size_t i = 0; i < a.size(); ++i) {
		const float d = a[i] - b[i];
		sum += d * d;
	}
	return std::sqrt(sum);
}

void ObjectClassifier::teach(const DetectedObject& obj, const std::string& label, const DetectionSettings& s) {
	const std::vector<float> f = featureVector(obj, s);

	for (auto& c : classes) {
		if (c.label != label) continue;
		if (c.prototype.size() != f.size()) { c.prototype = f; c.sampleCount = 1; return; }
		// running average keeps the prototype in the middle of all examples
		const float n = float(c.sampleCount);
		for (size_t i = 0; i < f.size(); ++i) c.prototype[i] = (c.prototype[i] * n + f[i]) / (n + 1.0f);
		c.sampleCount++;
		return;
	}
	ObjectClass c;
	c.label = label;
	c.prototype = f;
	c.sampleCount = 1;
	classes.push_back(c);
}

void ObjectClassifier::clear() { classes.clear(); }

void ObjectClassifier::classify(std::vector<DetectedObject>& objects, const DetectionSettings& s) {
	for (auto& o : objects) { o.label.clear(); o.classIndex = -1; }
	if (objects.empty()) return;

	if (hasClasses())      classifyByNearest(objects, s);
	else if (s.autoGroup)  classifyByGrouping(objects, s);
}

void ObjectClassifier::classifyByNearest(std::vector<DetectedObject>& objects, const DetectionSettings& s) const {
	for (auto& o : objects) {
		const std::vector<float> f = featureVector(o, s);
		float bestDist = 1e9f;
		int   best     = -1;
		for (size_t i = 0; i < classes.size(); ++i) {
			const float d = distance(f, classes[i].prototype);
			if (d < bestDist) { bestDist = d; best = int(i); }
		}
		if (best >= 0 && bestDist <= s.maxMatchDistance) {
			o.classIndex = best;
			o.label      = classes[best].label;
		} else {
			o.label = "unknown";
		}
	}
}

void ObjectClassifier::classifyByGrouping(std::vector<DetectedObject>& objects, const DetectionSettings& s) const {
	const int k = std::max(1, std::min(s.groupCount, int(objects.size())));
	if (k == 1) {
		for (auto& o : objects) { o.classIndex = 0; o.label = "group 1"; }
		return;
	}

	const std::vector<float> first = featureVector(objects[0], s);
	cv::Mat samples(int(objects.size()), int(first.size()), CV_32F);
	for (size_t r = 0; r < objects.size(); ++r) {
		const std::vector<float> f = featureVector(objects[r], s);
		for (size_t c = 0; c < f.size(); ++c) samples.at<float>(int(r), int(c)) = f[c];
	}

	cv::Mat labels, centres;
	const cv::TermCriteria crit(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 20, 0.001);
	cv::kmeans(samples, k, labels, crit, 3, cv::KMEANS_PP_CENTERS, centres);

	for (size_t r = 0; r < objects.size(); ++r) {
		const int g = labels.at<int>(int(r));
		objects[r].classIndex = g;
		objects[r].label      = "group " + std::to_string(g + 1);
	}
}

