// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    VideoFileSource.cpp
//  Author:  Kelvin Maritim
//  Purpose: Video file playback.
// ---------------------------------------------------------------------------
#include "VideoFileSource.h"

bool VideoFileSource::load(const std::string& path) {
	player.setUseTexture(false);
	player.setPixelFormat(OF_PIXELS_RGB);      // ask for plain RGB pixels
	if (!player.load(path)) {
		ofLogError("VideoFileSource") << "could not load " << path;
		return false;
	}
	player.setLoopState(OF_LOOP_NORMAL);
	player.play();
	name = ofFilePath::getFileName(path);
	return true;
}
