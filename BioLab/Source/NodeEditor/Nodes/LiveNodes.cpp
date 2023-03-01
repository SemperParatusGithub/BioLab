#include "LiveNodes.h"


Source::Source(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Source;
}
Source::~Source()
{
}

void Source::Render()
{
}
float Source::ProcessSample(float newSample)
{
	return newSample;
}


Scope::Scope(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Scope;
}
Scope::~Scope()
{
}

void Scope::Render()
{
	ImGui::Selectable("Output", false);
}
float Scope::ProcessSample(float newSample)
{
	this->Samples.PushBack(newSample);

	return newSample;
}

void Scope::ClearBuffer()
{
	this->Samples.Clear();
}