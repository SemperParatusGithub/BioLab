#include "PlotWindow.h"

#include <imgui.h>
#include <implot.h>

#include "Application.h"


PlotWindow::PlotWindow()
{
}
PlotWindow::~PlotWindow()
{
}

void PlotWindow::Render()
{
	if (!m_IsOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(m_Name.c_str(), &m_IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		auto* instance = Application::Instance();
		if (instance->m_Reading && instance->m_xValues.Size() != 0)
		{
			bool shouldPlot = instance->m_xValues.Size() != 0;
			float xMin = (float)instance->m_xValues.Back() - 10.5f;
			float xMax = (float)instance->m_xValues.Back() + 0.5f;
			ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
		}

		std::string plotName = "##" + m_Name;
		ImPlot::BeginPlot(plotName.c_str(), ImGui::GetContentRegionAvail());

		// Handle drag n drop interractions
		if (ImPlot::BeginDragDropTargetPlot())
		{
			const ImGuiPayload* scopePayload = ImGui::AcceptDragDropPayload("DND_SCOPE");
			if (scopePayload)
			{
				int id = *(int*)scopePayload->Data;
				LOG_INFO("Dropped Scope ID: %d", id);
				m_ScopeIDs.push_back(id);
			}

			const ImGuiPayload* signalPayload = ImGui::AcceptDragDropPayload("DND_SIGNAL");
			if (signalPayload)
			{
				int id = *(int*)signalPayload->Data;
				LOG_INFO("Dropped Signal ID: %d", id);
				m_SignalIDs.push_back(id);
			}
			ImPlot::EndDragDropTarget();
		}

		// Plot active signals
		for (auto id : m_SignalIDs)
		{
			auto* signal = Application::Instance()->GetSignalByID(id);
			if (signal == nullptr)
				continue;
			signal->Plot();

			if (ImPlot::BeginLegendPopup(signal->label.c_str()))
			{
				ImGui::SliderFloat("Thickness", &signal->thickness, 1.0f, 8.0f);
				ImGui::SliderFloat("Alpha", &signal->alpha, 0.0f, 1.0f);
				ImGui::ColorEdit3("Color", (float*)&signal->color);
				ImGui::Checkbox("Markers", &signal->markers);
				ImGui::Checkbox("Shaded", &signal->shaded);
				ImGui::SliderInt("Stride", &signal->stride, 1, 25);

				ImPlot::EndLegendPopup();
			}
		}
		for (auto id : m_ScopeIDs)
		{
			auto* scope = instance->GetScopeByID(id);
			if (scope == nullptr)
				continue;
		
			if (scope->Samples.Size() != 0)
			{
				ImPlot::PlotLine(scope->name.c_str(),
					instance->m_xValues.Data(), scope->Samples.Data(), scope->Samples.Size());
			}
		}
		ImPlot::EndPlot();
	}
	ImGui::End();
}
void PlotWindow::Clear()
{
	m_SignalIDs.clear();
	m_ScopeIDs.clear();
}