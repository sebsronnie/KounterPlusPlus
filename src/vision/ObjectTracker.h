// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ObjectTracker.h
//  Author:  Ronald Sebuhinja
//  Purpose: Gives objects in a video a stable id from frame to frame, so the
//           count does not flicker and we can count how many different
//           objects have passed through the picture in total.
// -----------------------------------------------------------------------------
#pragma once

#include <opencv2/core.hpp>
#include <vector>

#include "DetectedObject.h"
#include "DetectionSettings.h"

struct Track {
	int         id = 0;
	cv::Point2f position;
	int         framesSeen   = 0;   // how many frames it has been matched
	int         framesMissed = 0;   // consecutive frames without a match
	bool        counted      = false;
};

class ObjectTracker {
public:
	// Matches this frame's objects to existing tracks and writes trackId into them.
	void update(std::vector<DetectedObject>& objects, const DetectionSettings& s);

	void reset();
	int  activeCount() const;              // tracks currently visible
	int  totalCounted() const { return totalSeen; }
	const std::vector<Track>& getTracks() const { return tracks; }

private:
	std::vector<Track> tracks;
	int nextId    = 1;
	int totalSeen = 0;
};

