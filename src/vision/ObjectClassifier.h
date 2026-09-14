// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ObjectClassifier.h
//  Author:  Ronald Sebuhinja
//  Purpose: Tells objects apart. Two ways:
//             1. Teach by example: the user clicks an object and names it.
//                Later objects get the name of the closest taught example.
//             2. Automatic grouping: k-means puts similar objects together
//                when nothing has been taught yet.
// -----------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>

#include "DetectedObject.h"
#include "DetectionSettings.h"

struct ObjectClass {
	std::string        label;
	std::vector<float> prototype;     // average feature vector of the taught examples
	int                sampleCount = 0;
};

class ObjectClassifier {
public:
	// Remember this object as an example of "label". Teaching the same label
	// again refines the prototype (running average).
	void teach(const DetectedObject& obj, const std::string& label, const DetectionSettings& s);

	// Fills label / classIndex of every object.
	void classify(std::vector<DetectedObject>& objects, const DetectionSettings& s);

	void clear();
	bool hasClasses() const { return !classes.empty(); }
	const std::vector<ObjectClass>& getClasses() const { return classes; }

	// Numbers that describe an object, scaled so they can be compared.
	static std::vector<float> featureVector(const DetectedObject& obj, const DetectionSettings& s);

private:
	void classifyByNearest(std::vector<DetectedObject>& objects, const DetectionSettings& s) const;
	void classifyByGrouping(std::vector<DetectedObject>& objects, const DetectionSettings& s) const;
	static float distance(const std::vector<float>& a, const std::vector<float>& b);

	std::vector<ObjectClass> classes;
};

