// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    VideoFileSource.h
//  Author:  Kelvin Maritim
//  Purpose: Frames from a video file (mp4, mov) through ofVideoPlayer.
//           Stretch goal: built after camera support works.
// ---------------------------------------------------------------------------
#pragma once

#include "FrameSource.h"

class VideoFileSource : public FrameSource {
public:
	bool load(const std::string& path);

	void update() override { player.update(); }
	bool isFrameNew() const override { return player.isFrameNew(); }
	const ofPixels& getPixels() const override { return player.getPixels(); }
	int getWidth() const override  { return int(player.getWidth()); }
	int getHeight() const override { return int(player.getHeight()); }
	std::string getName() const override { return name; }
	bool isLive() const override { return true; }
	void close() override { player.close(); }

	void togglePause() { player.setPaused(!player.isPaused()); }

private:
	ofVideoPlayer player;
	std::string name;
};
