#include "Scope.h"


Scope::Scope(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->Name = nodeName;
	this->ID = nodeID;
	this->Position = position;
	this->Size = size;
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
	ImGui::Text("ID: %d", this->ID);
}