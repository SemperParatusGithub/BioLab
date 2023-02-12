#include "Source.h"


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