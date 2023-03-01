#include "ProcessingNodes.h"

#include <string>


Gain::Gain(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Gain;
}
Gain::~Gain()
{
}

void Gain::Render()
{
	float regionWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetNextItemWidth(regionWidth);
	ImGui::DragFloat("##Gain", &m_Gain, 0.05f);
}
float Gain::ProcessSample(float newSample)
{
	return newSample * m_Gain;
}

Signal Gain::ProcessSignal(const Signal& signal)
{
	Signal out = signal;

	for (float& y : out.yValues)
		y *= m_Gain;

	return out;
}



Offset::Offset(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Offset;
}
Offset::~Offset()
{
}

void Offset::Render()
{
	float regionWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetNextItemWidth(regionWidth);
	ImGui::DragFloat("##Offset", &m_Offset, 1.0f);
}
float Offset::ProcessSample(float newSample)
{
	return newSample + m_Offset;
}

Signal Offset::ProcessSignal(const Signal& signal)
{
	Signal out = signal;

	for (float& y : out.yValues)
		y += m_Offset;

	return out;
}



Absolute::Absolute(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Absolute;
}
Absolute::~Absolute()
{
}

void Absolute::Render()
{
	ImGui::Checkbox("Invert", &m_Invert);
}
float Absolute::ProcessSample(float newSample)
{
	return std::abs(newSample) * (m_Invert ? -1.0f : 1.0f);
}

Signal Absolute::ProcessSignal(const Signal& signal)
{
	Signal out = signal;

	for (float& y : out.yValues)
		y = abs(y) * (m_Invert ? -1.0f : 1.0f);

	return out;
}



Filter::Filter(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Filter;

	m_Filter = IIRFilter({ 0.5f, -0.5f, 0.5f }, { 1.0f, 2.0f, 3.0f });
}
Filter::~Filter()
{
}

void Filter::Render()
{
	float regionWidth = ImGui::GetContentRegionAvail().x;

	auto [B, A] = m_Filter.GetCoefficients();
	
	for (int i = 0; i < B.size(); i++)
	{
		std::string label;
		float value;
	
		label = "B" + std::to_string(i);
		value = B[i];
	
		ImGui::SetNextItemWidth(regionWidth / 2.3f - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::DragFloat(label.c_str(), &value, 0.1f))
			B[i] = value;
	
		ImGui::SameLine();
	
		label = "A" + std::to_string(i);
		value = A[i];
	
		ImGui::SetNextItemWidth(regionWidth / 2.5f - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::DragFloat(label.c_str(), &value, 0.1f))
			A[i] = value;
	}
	
	if (ImGui::Button("Clear buffers"))
		m_Filter.ClearBuffers();


	m_Filter = IIRFilter(B, A);
}
float Filter::ProcessSample(float newSample)
{
	return m_Filter.ProcessSample(newSample);
}

Signal Filter::ProcessSignal(const Signal& signal)
{
	return m_Filter.ProcessSignal(signal);
}
