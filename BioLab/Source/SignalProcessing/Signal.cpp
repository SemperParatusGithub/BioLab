#include "Signal.h"

#include <imgui.h>
#include <implot.h>

#include <algorithm>


void Signal::Plot()
{
	if (stride >= xValues.size() / 2)
		return;

	ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, alpha);

	ImPlot::SetNextLineStyle(ImVec4(color.x, color.y, color.z, color.w), thickness);
	if (shaded)
		ImPlot::PlotShaded(label.c_str(), xValues.data(), yValues.data(), xValues.size());

	if (markers) 
		ImPlot::SetNextMarkerStyle(ImPlotMarker_Square);

	ImPlot::PlotLine(label.c_str(), xValues.data(), yValues.data(), xValues.size() / stride, 0, 0, stride * sizeof(float));

	ImPlot::PopStyleVar();
}