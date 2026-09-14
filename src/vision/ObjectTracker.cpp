// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ObjectTracker.cpp
//  Author:  Ronald Sebuhinja
//  Purpose: Greedy nearest-centroid tracking.
// -----------------------------------------------------------------------------
#include "ObjectTracker.h"

#include <algorithm>
#include <cmath>

namespace {
struct Candidate {
	float  dist;
	size_t track;
	size_t object;
};
}

void ObjectTracker::update(std::vector<DetectedObject>& objects, const DetectionSettings& s) {
	// 1. Every possible (track, object) pair that is close enough, nearest first.
	std::vector<Candidate> candidates;
	for (size_t t = 0; t < tracks.size(); ++t) {
		for (size_t o = 0; o < objects.size(); ++o) {
			const cv::Point2f d = objects[o].centroid - tracks[t].position;
			const float dist = std::sqrt(d.x * d.x + d.y * d.y);
			if (dist <= s.trackMaxDistance) candidates.push_back({dist, t, o});
		}
	}
	std::sort(candidates.begin(), candidates.end(),
			  [](const Candidate& a, const Candidate& b) { return a.dist < b.dist; });

	// 2. Take the closest pairs first; each track and object may be used once.
	std::vector<bool> trackUsed(tracks.size(), false), objectUsed(objects.size(), false);
	for (const auto& c : candidates) {
		if (trackUsed[c.track] || objectUsed[c.object]) continue;
		trackUsed[c.track] = objectUsed[c.object] = true;

		Track& tr = tracks[c.track];
		tr.position = objects[c.object].centroid;
		tr.framesSeen++;
		tr.framesMissed = 0;
		objects[c.object].trackId = tr.id;
	}

	// 3. Unmatched objects start new tracks.
	for (size_t o = 0; o < objects.size(); ++o) {
		if (objectUsed[o]) continue;
		Track tr;
		tr.id = nextId++;
		tr.position = objects[o].centroid;
		tr.framesSeen = 1;
		tracks.push_back(tr);
		objects[o].trackId = tr.id;
	}

	// 4. Unmatched tracks age; old ones are dropped. A track is only counted
	//    once it has survived a few frames, which filters out flicker.
	for (size_t t = 0; t < tracks.size(); ++t) {
		if (t < trackUsed.size() && !trackUsed[t]) tracks[t].framesMissed++;
		if (!tracks[t].counted && tracks[t].framesSeen >= s.trackMinFrames) {
			tracks[t].counted = true;
			totalSeen++;
		}
	}
	tracks.erase(std::remove_if(tracks.begin(), tracks.end(),
								[&](const Track& tr) { return tr.framesMissed > s.trackMaxMissed; }),
				 tracks.end());
}

void ObjectTracker::reset() {
	tracks.clear();
	nextId    = 1;
	totalSeen = 0;
}

int ObjectTracker::activeCount() const {
	int n = 0;
	for (const auto& t : tracks) if (t.framesMissed == 0 && t.counted) ++n;
	return n;
}

