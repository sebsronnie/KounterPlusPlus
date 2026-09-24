// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    Segmenter.h
//  Author:  Ronald K. Sebuhinja
//  Purpose: Turns a frame into a binary mask (white = object, black = background).
//           Each way of thresholding is its own strategy class; the Segmenter
//           owns one of each and picks the right one from the settings.
// -----------------------------------------------------------------------------
#pragma once

#include <opencv2/core.hpp>
#include <memory>
#include <string>
#include <vector>

#include "DetectionSettings.h"

// Base class of the strategy family (inheritance + polymorphism).
class SegmentationStrategy {
public:
	virtual ~SegmentationStrategy() = default;

	// gray: 8-bit single channel, already blurred.
	// hsv:  8-bit 3 channel HSV of the same frame (empty for grayscale input).
	// mask: output, 8-bit single channel, 255 = object.
	virtual void apply(const cv::Mat& gray, const cv::Mat& hsv,
					   const DetectionSettings& s, cv::Mat& mask) const = 0;

	virtual SegmentationMode mode() const = 0;
	virtual std::string name() const = 0;
};

class OtsuStrategy : public SegmentationStrategy {
public:
	void apply(const cv::Mat& gray, const cv::Mat& hsv, const DetectionSettings& s, cv::Mat& mask) const override;
	SegmentationMode mode() const override { return SegmentationMode::AutoOtsu; }
	std::string name() const override { return "Auto (Otsu)"; }
};

class ManualThresholdStrategy : public SegmentationStrategy {
public:
	void apply(const cv::Mat& gray, const cv::Mat& hsv, const DetectionSettings& s, cv::Mat& mask) const override;
	SegmentationMode mode() const override { return SegmentationMode::ManualThreshold; }
	std::string name() const override { return "Manual threshold"; }
};

class AdaptiveStrategy : public SegmentationStrategy {
public:
	void apply(const cv::Mat& gray, const cv::Mat& hsv, const DetectionSettings& s, cv::Mat& mask) const override;
	SegmentationMode mode() const override { return SegmentationMode::Adaptive; }
	std::string name() const override { return "Adaptive"; }
};

class ColourPickStrategy : public SegmentationStrategy {
public:
	void apply(const cv::Mat& gray, const cv::Mat& hsv, const DetectionSettings& s, cv::Mat& mask) const override;
	SegmentationMode mode() const override { return SegmentationMode::ColourPick; }
	std::string name() const override { return "Colour pick"; }
};

// Fifth option aimed at fixing the AutoOtsu  issue with inappropriate threshold
// Otsu draws ONE line through the histogram. A scene with a bright background,
// bright objects and dark objects has three populations, so two of them always
// land on the same side of that line. This strategy asks a different question:
// not "is this pixel dark?" but "is this pixel unlike the background?".
class BackgroundRemoveStrategy : public SegmentationStrategy {
public:
	void apply(const cv::Mat& gray, const cv::Mat& hsv, const DetectionSettings& s, cv::Mat& mask) const override;
	SegmentationMode mode() const override { return SegmentationMode::BackgroundRemove; }
	std::string name() const override { return "Background removal"; }
};

// The Segmenter composes the strategies and the clean-up steps.
class Segmenter {
public:
	Segmenter();

	// rgb: 8-bit, 1 or 3 channels (RGB order, as openFrameworks gives it).
	// mask: output, same size as rgb, 8-bit single channel.
	void segment(const cv::Mat& rgb, const DetectionSettings& s, cv::Mat& mask);

	std::string strategyName(SegmentationMode m) const;

private:
	const SegmentationStrategy& strategyFor(SegmentationMode m) const;

	void cleanUp(cv::Mat& mask, const DetectionSettings& s) const;
	void fillHoles(cv::Mat& mask) const;
	void splitTouching(cv::Mat& mask, const cv::Mat& rgb, const DetectionSettings& s) const;

	std::vector<std::unique_ptr<SegmentationStrategy>> strategies;   // composition

	// Scratch buffers, kept as members so they are not re-allocated every frame.
	cv::Mat gray, blurred, hsv;
};

