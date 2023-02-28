#include "DefaultPlotWindow.h"

#include <imgui.h>
#include <implot.h>

#include "Application.h"
#include "IconsMaterialDesign.h"


DefaultPlotWindow::DefaultPlotWindow()
{
}
DefaultPlotWindow::~DefaultPlotWindow()
{
}

void DefaultPlotWindow::Render()
{
	if (!m_IsOpen)
		return;

	if (ImGui::Begin(ICON_MD_INSERT_CHART" Default Plot Window", &m_IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		auto* instance = Application::Instance();
		bool shouldPlot = instance->m_LiveValuesX.Size() != 0;
		float xMin = 0.0f; 
		float xMax = 1.0f;

		if (shouldPlot)
		{
			xMin = (float)instance->m_LiveValuesX.Back() - 10.5f;
			xMax = (float)instance->m_LiveValuesX.Back() + 0.5f;
		}
		
		if (ImPlot::BeginSubplots("", 3, 1, ImGui::GetContentRegionAvail()))
		{
			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("CH1", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", instance->m_LiveValuesX.Data(), instance->m_LiveValuesCH1.Data(), instance->m_LiveValuesX.Size());
			}
			ImPlot::EndPlot();

			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("CH2", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", instance->m_LiveValuesX.Data(), instance->m_LiveValuesCH2.Data(), instance->m_LiveValuesX.Size());
			}
			ImPlot::EndPlot();

			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("CH3", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", instance->m_LiveValuesX.Data(), instance->m_LiveValuesCH3.Data(), instance->m_LiveValuesX.Size());
			}
			ImPlot::EndPlot();
		}
		ImPlot::EndSubplots();
	}
	ImGui::End();
}