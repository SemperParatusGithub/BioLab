#include "OfflineNodes.h"

#include "Util/FileUtils.h"
#include "Application.h"

#include "SignalProcessing/SignalProcessing.h"

#include <numeric>


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
		if (ImGui::Button(ICON_MD_DELETE, ImVec2(30, 23)))
			m_SignalID = -1;
	}
	else {
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 35));
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


FourierTransform::FourierTransform(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::FourierTransform;
}
FourierTransform::~FourierTransform()
{
}

void FourierTransform::Render()
{
	ImGui::TextUnformatted("FFT");

	ImGui::SetNextItemWidth(-1);
	ImGui::DragInt("fs", &m_fs);
}
Signal FourierTransform::ProcessSignal(const Signal& signal)
{
	Signal returnSignal = signal;

	//returnSignal.yValues = SignalProcessing::DFT(signal.yValues);
	returnSignal.yValues = SignalProcessing::FFT(signal.yValues);
	int fs = 2000;
	for (int i = 0; i < returnSignal.xValues.size(); i++)
	{
		int s = returnSignal.xValues.size();
		double val = (double)i / double(s) * fs;
		returnSignal.xValues[i] = val;
	}

	return returnSignal;
}


RemoveDC::RemoveDC(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::RemoveDC;
}
RemoveDC::~RemoveDC()
{
}
void RemoveDC::Render()
{
}
Signal RemoveDC::ProcessSignal(const Signal& signal)
{
	Signal returnSignal = signal;

	float average = std::accumulate(signal.yValues.begin(), signal.yValues.end(), 0.0) / signal.yValues.size();

	for (auto& val : returnSignal.yValues)
		val -= average;

	return returnSignal;
}


Normalize::Normalize(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Normalize;
}
Normalize::~Normalize()
{
}
void Normalize::Render()
{
}
Signal Normalize::ProcessSignal(const Signal& signal)
{
	Signal returnSignal = signal;

	float min = *std::min_element(signal.yValues.begin(), signal.yValues.end());
	float max = *std::max_element(signal.yValues.begin(), signal.yValues.end());

	for (auto& val : returnSignal.yValues)
		val = (val - min) / (max - min);

	return returnSignal;
}