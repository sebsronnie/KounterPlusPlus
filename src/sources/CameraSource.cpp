// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    CameraSource.cpp
//  Author:  Kelvin Maritim
//  Purpose: Webcam setup.
// ---------------------------------------------------------------------------
#include "CameraSource.h"

bool CameraSource::open(int id, int width, int height) {
	deviceId = id;
	grabber.setDeviceID(deviceId);
	grabber.setDesiredFrameRate(30);
	grabber.setUseTexture(false);              // we upload our own texture

	// Ask for the size we want; the driver may give us something else,
	// which is fine because the pipeline reads the real size every frame.
	if (!grabber.setup(width, height)) {
		ofLogError("CameraSource") << "could not open camera " << deviceId;
		return false;
	}
	ofLogNotice("CameraSource") << "opened camera " << deviceId << " at "
	                            << grabber.getWidth() << "x" << grabber.getHeight();
	return true;
}
