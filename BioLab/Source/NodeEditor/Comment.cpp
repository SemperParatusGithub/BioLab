#include "Comment.h"

#include <imgui.h>


Comment::Comment(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->Name = nodeName;
	this->ID = nodeID;
	this->Position = position;
	this->Size = size;
}
Comment::~Comment()
{
}

NodeType Comment::GetNodeType() const
{
	return NodeType::Comment;
}
void Comment::Render()
{
	namespace editor = ax::NodeEditor;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.75f);
	editor::PushStyleColor(editor::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
	editor::PushStyleColor(editor::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
	editor::BeginNode(this->ID);

	ImGui::PushID(this->ID.AsPointer());
	ImGui::Text("%s (ID: %d)", this->Name.c_str(), this->ID.Get());
	editor::Group(ImVec2{ this->Size.x, this->Size.y });
	ImGui::PopID();

	editor::EndNode();
	editor::PopStyleColor(2);
	ImGui::PopStyleVar();
}