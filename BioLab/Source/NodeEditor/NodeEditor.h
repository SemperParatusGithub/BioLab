#pragma once
#include "Node.h"


struct Link
{
	ax::NodeEditor::LinkId ID;

	ax::NodeEditor::PinId StartPinID;
	ax::NodeEditor::PinId EndPinID;

	ImColor Color = ImColor(255, 255, 255);
};

struct NodeEditorConfig
{
};

class NodeEditor
{
public:
	NodeEditor(const NodeEditorConfig& config = NodeEditorConfig{});
	~NodeEditor();

	void Render(const ImVec2& size);
	void ShowDebugWindow();

private:
	ax::NodeEditor::NodeId GetNextNodeID();
	ax::NodeEditor::LinkId GetNextLinkID();
	ax::NodeEditor::PinId GetNextPinID();

	bool CanCreateLink(Pin* a, Pin* b);
	void HandleLinks();


private:
	ax::NodeEditor::EditorContext* m_EditorContext;
	ImFont* m_Font;

	std::vector<Node*> m_Nodes;		// TODO: Delete me afterwards
	std::vector<Link> m_Links;

	int m_CurrentNodeID = 10000;
	int m_CurrentLinkID = 20000;
	int m_CurrentPinID = 30000;

	NodeEditorConfig m_Config;
};