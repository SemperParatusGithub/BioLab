#include "Source.h"


Source::Source(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	SetNodeID(nodeID);
	SetName(nodeName);
	SetPosition(position);
	SetSize(size);
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
	ImGui::Text("Source Content");
	ImGui::Text("Source Content");
	ImGui::Text("Source Content");
}