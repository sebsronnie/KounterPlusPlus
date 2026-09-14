// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    CameraSource.h
//  Author:  Kelvin Maritim
//  Purpose: Live frames from the webcam through ofVideoGrabber.
// ---------------------------------------------------------------------------
#pragma once

#include "FrameSource.h"

class CameraSource : public FrameSource {
public:
	// Opens the camera. Returns false if no camera could be opened.
	bool open(int deviceId = 0, int width = 1280, int height = 720);

	void update() override { grabber.update(); }
	bool isFrameNew() const override { return grabber.isFrameNew(); }
	const ofPixels& getPixels() const override { return grabber.getPixels(); }
	int getWidth() const override  { return int(grabber.getWidth()); }
	int getHeight() const override { return int(grabber.getHeight()); }
	std::string getName() const override { return "Camera " + ofToString(deviceId); }
	bool isLive() const override { return true; }
	void close() override { grabber.close(); }

private:
	ofVideoGrabber grabber;
	int deviceId = 0;
};
