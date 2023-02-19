#include "PlotWindow.h"

#include <imgui.h>
#include <implot.h>


PlotWindow::PlotWindow()
{
}
PlotWindow::~PlotWindow()
{
}

void PlotWindow::Begin()
{
	if (!m_IsOpen)
		return;
	if (ImGui::Begin(m_Name.c_str(), &m_IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		std::string plotName = "##" + m_Name;
		ImPlot::BeginPlot(plotName.c_str(), ImGui::GetContentRegionAvail());

		if (ImPlot::BeginDragDropTargetPlot())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SCOPE");
			if (payload)
			{
				int id = *(int*)payload->Data;
				LOG_INFO("DragNDrop ID: %d", id);
				m_SignalIDs.push_back(id);
			}
			ImPlot::EndDragDropTarget();
		}
	}
}

void PlotWindow::End()
{
	if (!m_IsOpen)
		return;

	ImPlot::EndPlot();

	ImGui::End();
}