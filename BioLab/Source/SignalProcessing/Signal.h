#pragma once
#include "Core.h"

#include "Util/Buffer.h"

#include <imgui.h>


struct Signal
{
public:
	SignalID id = -1;
	std::string label = "Unnamed";
	ImVec4 color;
	std::vector<float> xValues, yValues;

	bool markers = false;
	bool shaded = false;
	float thickness = 2.0f;
	float alpha = 0.2f;
	i32 stride = 1;

public:
	void Plot();	// Note: Must be called between BeginPlot and EndPlot!!!
};