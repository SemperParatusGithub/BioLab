#include "Comment.h"

#include <imgui.h>


Comment::Comment(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Comment;
}
Comment::~Comment()
{
}

void Comment::Render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.75f);
	ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
	ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
	ax::NodeEditor::BeginNode(this->id);

	ImGui::PushID(this->id.AsPointer());
	ImGui::Spacing();
	ImGui::Text("%s (ID: %d)", this->name.c_str(), this->id.Get());
	ax::NodeEditor::Group(ImVec2{ this->size.x, this->size.y });
	ImGui::PopID();

	ax::NodeEditor::EndNode();
	ax::NodeEditor::PopStyleColor(2);
	ImGui::PopStyleVar();
}