#include "Comment.h"

#include <imgui.h>


Comment::Comment(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	SetNodeID(nodeID);
	SetName(nodeName);
	SetPosition(position);
	SetSize(size);
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
	editor::BeginNode(m_NodeID);

	ImGui::PushID(m_NodeID.AsPointer());
	ImGui::Text("%s (ID: %d)", m_NodeName.c_str(), m_NodeID.Get());
	editor::Group(ImVec2{ m_Size.x, m_Size.y });
	ImGui::PopID();

	editor::EndNode();
	editor::PopStyleColor(2);
	ImGui::PopStyleVar();
}