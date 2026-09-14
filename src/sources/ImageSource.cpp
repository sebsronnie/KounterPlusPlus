// ---------------------------------------------------------------------------
//  Object Counter  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ImageSource.cpp
//  Author:  Kelvin Maritim
//  Purpose: Loads a still image and normalises it to RGB or grayscale.
// ---------------------------------------------------------------------------
#include "ImageSource.h"

bool ImageSource::load(const std::string& path) {
	// setUseTexture(false): we draw through our own texture, so the image
	// itself does not need one. Saves GPU memory and one upload.
	image.setUseTexture(false);
	if (!image.load(path)) {
		ofLogError("ImageSource") << "could not load " << path;
		return false;
	}

	// PNGs often carry an alpha channel we do not need. Grayscale files stay
	// grayscale so the black-and-white path of the pipeline gets exercised.
	if (image.getImageType() == OF_IMAGE_COLOR_ALPHA) image.setImageType(OF_IMAGE_COLOR);

	name  = ofFilePath::getFileName(path);
	fresh = true;
	return true;
}

bool ImageSource::isFrameNew() const {
	const bool wasFresh = fresh;
	fresh = false;
	return wasFresh;
}
