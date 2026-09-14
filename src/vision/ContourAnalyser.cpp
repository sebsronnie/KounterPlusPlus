// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ContourAnalyser.cpp
//  Author:  Ronald Sebuhinja
//  Purpose: Contour extraction and per-object measurements.
// -----------------------------------------------------------------------------
#include "ContourAnalyser.h"

#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

void ContourAnalyser::analyse(const cv::Mat& mask, const cv::Mat& rgb,
							  const DetectionSettings& s, std::vector<DetectedObject>& objects) {
	objects.clear();

	// OpenCV 3 modifies the input of findContours, OpenCV 4 does not.
	// Working on a copy keeps us safe on both.
	mask.copyTo(maskCopy);
	cv::findContours(maskCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	if (rgb.channels() == 3) cv::cvtColor(rgb, hsv, cv::COLOR_RGB2HSV);
	else                     hsv.release();

	objects.reserve(contours.size());
	for (auto& c : contours) {
		const double area = cv::contourArea(c);
		if (area < s.minArea) continue;
		if (s.maxArea > 0.0f && area > s.maxArea) continue;

		DetectedObject obj;
		obj.contour = std::move(c);
		obj.area    = area;
		measure(obj, rgb, hsv);
		objects.push_back(std::move(obj));
	}

	// Largest first, so object number 1 is always the biggest one on screen.
	std::sort(objects.begin(), objects.end(),
			  [](const DetectedObject& a, const DetectedObject& b) { return a.area > b.area; });
}

void ContourAnalyser::measure(DetectedObject& obj, const cv::Mat& rgb, const cv::Mat& hsvFrame) {
	const auto& c = obj.contour;

	obj.bbox      = cv::boundingRect(c);
	obj.perimeter = cv::arcLength(c, true);

	const cv::Moments m = cv::moments(c);
	if (m.m00 > 0.0) obj.centroid = cv::Point2f(float(m.m10 / m.m00), float(m.m01 / m.m00));
	else             obj.centroid = cv::Point2f(float(obj.bbox.x + obj.bbox.width / 2.0),
												float(obj.bbox.y + obj.bbox.height / 2.0));

	// circularity = 4*pi*area / perimeter^2 : 1 for a circle, lower for anything else
	obj.circularity = obj.perimeter > 0.0 ? std::min(1.0, 4.0 * CV_PI * obj.area / (obj.perimeter * obj.perimeter)) : 0.0;

	const double longSide  = std::max(obj.bbox.width, obj.bbox.height);
	const double shortSide = std::min(obj.bbox.width, obj.bbox.height);
	obj.aspectRatio = longSide > 0.0 ? shortSide / longSide : 1.0;

	std::vector<cv::Point> hull;
	cv::convexHull(c, hull);
	const double hullArea = cv::contourArea(hull);
	obj.solidity = hullArea > 0.0 ? std::min(1.0, obj.area / hullArea) : 0.0;

	// Average colour inside the contour only (not the whole bounding box).
	// We draw the contour into a small mask the size of the bounding box.
	objectMask = cv::Mat::zeros(obj.bbox.size(), CV_8UC1);
	std::vector<std::vector<cv::Point>> one(1, c);
	cv::drawContours(objectMask, one, 0, cv::Scalar(255), cv::FILLED, cv::LINE_8, cv::noArray(), 0, -obj.bbox.tl());

	if (hsvFrame.empty()) {
		const cv::Scalar mean = cv::mean(rgb(obj.bbox), objectMask);
		obj.meanHue = 0.0; obj.meanSaturation = 0.0; obj.meanValue = mean[0];
		return;
	}

	// Hue is an angle on a circle (red sits at both 0 and 180), so a plain
	// average is wrong for red objects. We average the direction instead.
	const cv::Mat roi = hsvFrame(obj.bbox);
	double sumCos = 0.0, sumSin = 0.0, sumSat = 0.0, sumVal = 0.0;
	int count = 0;
	for (int y = 0; y < roi.rows; ++y) {
		const cv::Vec3b* px = roi.ptr<cv::Vec3b>(y);
		const uchar*     mk = objectMask.ptr<uchar>(y);
		for (int x = 0; x < roi.cols; ++x) {
			if (!mk[x]) continue;
			const double angle = px[x][0] * (CV_PI / 90.0);     // 0..180 -> 0..2pi
			sumCos += std::cos(angle);
			sumSin += std::sin(angle);
			sumSat += px[x][1];
			sumVal += px[x][2];
			++count;
		}
	}
	if (count == 0) return;
	double hue = std::atan2(sumSin, sumCos) * (90.0 / CV_PI);   // back to 0..180
	if (hue < 0.0) hue += 180.0;
	obj.meanHue        = hue;
	obj.meanSaturation = sumSat / count;
	obj.meanValue      = sumVal / count;
}

