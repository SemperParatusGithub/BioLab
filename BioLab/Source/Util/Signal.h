#pragma once
#include "Core.h"

#include <imgui.h>
#include <implot.h>


struct Signal
{
	int id = -1;
	std::string label = "Unnamed";
	ImVec4 color;
	std::vector<float> xValues, yValues;

	bool markers = false;
	bool shaded = false;
	float thickness = 2.0f;
	float alpha = 0.2f;

	// Note: Must be called between BeginPlot and EndPlot!!!
	void Plot()
	{
		ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, alpha);

		ImPlot::SetNextLineStyle(color, thickness);
		if (markers) ImPlot::SetNextMarkerStyle(ImPlotMarker_Square);
		if (shaded)
			ImPlot::PlotShaded(label.c_str(), xValues.data(), yValues.data(), xValues.size());
		else
			ImPlot::PlotLine(label.c_str(), xValues.data(), yValues.data(), xValues.size());

		ImPlot::PopStyleVar();
	}
};