// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    FrameSource.h
//  Author:  Kelvin Maritim
//  Purpose: Base class for anything that can give us a picture: a file,
//           the webcam or a video. The app only ever talks to this class,
//           so it does not care where the pixels come from.
//           FROZEN after Day 1.
// ---------------------------------------------------------------------------
#pragma once

#include "ofMain.h"
#include <string>

class FrameSource {
public:
	virtual ~FrameSource() = default;

	// Call once per frame from ofApp::update(). Live sources grab a new frame.
	virtual void update() = 0;

	// True when getPixels() holds something we have not processed yet.
	virtual bool isFrameNew() const = 0;

	// The current picture. Valid until the next update().
	virtual const ofPixels& getPixels() const = 0;

	virtual int getWidth() const  = 0;
	virtual int getHeight() const = 0;

	// Shown in the HUD, e.g. "oranges.jpg" or "Camera 0".
	virtual std::string getName() const = 0;

	// Live sources (camera, video) deliver frames continuously; the app then
	// runs the tracker and re-processes every frame.
	virtual bool isLive() const { return false; }

	// Release the camera / file. Called from ofApp::exit() and when switching.
	virtual void close() {}

	bool isReady() const { return getWidth() > 0 && getHeight() > 0; }
};
