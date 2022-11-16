#include "Scope.h"


Scope::Scope(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	SetNodeID(nodeID);
	SetName(nodeName);
	SetPosition(position);
	SetSize(size);
}
Scope::~Scope()
{
}

NodeType Scope::GetNodeType() const
{
	return NodeType::Scope;
}
void Scope::Render()
{
	ImGui::Text("Scope Content");
	ImGui::Text("Scope Content");
	ImGui::Text("Scope Content");
}