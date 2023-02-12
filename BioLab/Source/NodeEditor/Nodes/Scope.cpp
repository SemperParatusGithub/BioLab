#include "Scope.h"


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
	ImGui::Text("ID: %d", this->id);
	ImGui::Text("Buffer Size: %d", this->Samples.Size());
	ImGui::Button("Drag n drop source");
}

float Scope::ProcessSample(float newSample)
{
	this->Samples.PushBack(newSample);

	return newSample;
}