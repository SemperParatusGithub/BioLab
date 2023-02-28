#include "PostProcessingNodes.h"

#include "Util/FileUtils.h"
#include "Application.h"


InputSignal::InputSignal(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::InputSignal;
}
InputSignal::~InputSignal()
{
}

void InputSignal::Render()
{
	auto* instance = Application::Instance();
	auto* signal = instance->GetSignalByID(m_SignalID);
	if (signal != nullptr)
	{
		ImPlot::ItemIcon(signal->color); ImGui::SameLine();
		ImGui::TextUnformatted(signal->label.c_str());
	}
	else {
		ImGui::Button("target");
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SIGNAL");
			if (payload)
			{
				int id = *(int*)payload->Data;
				LOG_INFO("Signal ID: %d", id);

				m_SignalID = id;
			}
			ImGui::EndDragDropTarget();
		}
	}
}

Signal InputSignal::ProcessSignal(const Signal& signal)
{
	return signal;
}


OutputSignal::OutputSignal(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::OutputSignal;
}
OutputSignal::~OutputSignal()
{
}

void OutputSignal::Render()
{
	std::string label = "##" + std::to_string(m_Signal.id);
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText(label.c_str(), m_SignalName, 128);
}

Signal OutputSignal::ProcessSignal(const Signal& signal)
{
	m_Signal = signal;
	m_Signal.label = std::string(m_SignalName);
	m_Signal.id = FileUtils::GetNextSignalID();
	m_Signal.color = FileUtils::GetNextColor();

	return signal;
}