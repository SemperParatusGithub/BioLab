#include "LiveWindow.h"

#include <imgui.h>
#include <implot.h>

#include "Application.h"
#include "IconsMaterialDesign.h"


LiveWindow::LiveWindow()
{
}
LiveWindow::~LiveWindow()
{
}

void LiveWindow::ClearBuffers()
{
	m_xValues.Clear();

	m_Channel1.Clear();
	m_Channel2.Clear();
	m_Channel3.Clear();
}

void LiveWindow::AddNewSample(const Vector4f& sample)
{
	m_xValues.PushBack(sample.x);

	m_Channel1.PushBack(sample.y);
	m_Channel2.PushBack(sample.z);
	m_Channel3.PushBack(sample.w);
}

void LiveWindow::Render()
{
	if (!m_IsOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(ICON_MD_INSERT_CHART" Live Window", &m_IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		auto* instance = Application::Instance();
		bool shouldPlot = m_xValues.Size() != 0;
		float xMin = 0.0f; 
		float xMax = 1.0f;

		if (shouldPlot)
		{
			xMin = m_xValues.Back() - 10.5f;
			xMax = m_xValues.Back() + 0.5f;
		}
		
		if (ImPlot::BeginSubplots("", 3, 1, ImGui::GetContentRegionAvail()))
		{
			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("CH1", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", m_xValues.Data(), m_Channel1.Data(), m_xValues.Size());
			}
			ImPlot::EndPlot();

			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("CH2", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", m_xValues.Data(), m_Channel2.Data(), m_xValues.Size());
			}
			ImPlot::EndPlot();

			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("CH3", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", m_xValues.Data(), m_Channel3.Data(), m_xValues.Size());
			}
			ImPlot::EndPlot();
		}
		ImPlot::EndSubplots();
	}
	ImGui::End();
}