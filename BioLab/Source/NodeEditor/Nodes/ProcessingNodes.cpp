#include "ProcessingNodes.h"


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