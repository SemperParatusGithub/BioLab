#include "Source.h"


Source::Source(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->Name = nodeName;
	this->ID = nodeID;
	this->Position = position;
	this->Size = size;
}
Source::~Source()
{
}

NodeType Source::GetNodeType() const
{
	return NodeType::Source;
}
void Source::Render()
{
	ImGui::Text("ID: %d", this->ID);
}