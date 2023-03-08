#include "GoldbergerWindow.h"

#include "LiveWindow.h"

#include <imgui.h>
#include <implot.h>

#include "Application.h"
#include "IconsMaterialDesign.h"


GoldbergerWindow::GoldbergerWindow()
{
}

GoldbergerWindow::~GoldbergerWindow()
{
}

void GoldbergerWindow::ClearBuffers()
{
	m_AVR.Clear();
	m_AVL.Clear();
	m_AVF.Clear();
}
void GoldbergerWindow::AddNewSample(const Vector4f& sample)
{
	m_AVR.PushBack(-(sample.y + sample.z) / 2.0f);
	m_AVL.PushBack((sample.y - sample.w) / 2.0f);
	m_AVF.PushBack((sample.z + sample.w) / 2.0f);
}

void GoldbergerWindow::Render()
{
	if (!m_IsOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(ICON_MD_INSERT_CHART" Goldberger", &m_IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		auto* instance = Application::Instance();
		bool shouldPlot = instance->m_xValues.Size() != 0;
		float xMin = 0.0f;
		float xMax = 1.0f;

		if (shouldPlot)
		{
			xMin = instance->m_xValues.Back() - 10.5f;
			xMax = instance->m_xValues.Back() + 0.5f;
		}

		if (ImPlot::BeginSubplots("", 3, 1, ImGui::GetContentRegionAvail()))
		{
			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("aVR", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", instance->m_xValues.Data(), m_AVR.Data(), instance->m_xValues.Size());
			}
			ImPlot::EndPlot();

			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("aVL", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", instance->m_xValues.Data(), m_AVL.Data(), instance->m_xValues.Size());
			}
			ImPlot::EndPlot();

			ImPlot::SetNextAxisLimits(ImAxis_Y1, -1000, 5000, ImGuiCond_FirstUseEver);
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
			ImPlot::BeginPlot("aVF", 0, 0, ImGui::GetContentRegionAvail());
			if (shouldPlot)
			{
				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("data", instance->m_xValues.Data(), m_AVF.Data(), instance->m_xValues.Size());
			}
			ImPlot::EndPlot();
		}
		ImPlot::EndSubplots();
	}
	ImGui::End();
}