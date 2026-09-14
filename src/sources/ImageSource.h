// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ImageSource.h
//  Author:  Kelvin Maritim
//  Purpose: A still picture loaded from disk (jpg, png, bmp...).
// ---------------------------------------------------------------------------
#pragma once
#include "FrameSource.h"

class ImageSource : public FrameSource {
public:
	// Returns false if the file could not be read.
	bool load(const std::string& path);

	void update() override {}                        // a still image never changes
	bool isFrameNew() const override;                 // true exactly once after load
	const ofPixels& getPixels() const override { return image.getPixels(); }
	int getWidth() const override  { return int(image.getWidth()); }
	int getHeight() const override { return int(image.getHeight()); }
	std::string getName() const override { return name; }

private:
	ofImage image;
	std::string name;
	mutable bool fresh = false;                       // mutable: isFrameNew() is const but clears it
};
