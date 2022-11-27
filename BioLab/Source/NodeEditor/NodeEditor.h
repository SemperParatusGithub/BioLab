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

	void Render();
	void ShowDebugWindow();

	void Flow()
	{
		ax::NodeEditor::SetCurrentEditor(m_EditorContext);

		for (auto& link : m_Links)
			ax::NodeEditor::Flow(link.ID);

		ax::NodeEditor::SetCurrentEditor(nullptr);
	}

private:
	ax::NodeEditor::NodeId GetNextNodeID();
	ax::NodeEditor::LinkId GetNextLinkID();
	ax::NodeEditor::PinId GetNextPinID();

	bool IsPinConnected(ax::NodeEditor::PinId id)
	{
		for (auto& link : m_Links)
			if (link.StartPinID == id || link.EndPinID == id)
				return true;

		return false;
	}

	Pin FindPin(ax::NodeEditor::PinId id)
	{
		for (auto* node : m_Nodes)
		{
			if (node->InputPin.ID == id)
				return node->InputPin;
			if (node->OutputPin.ID == id)
				return node->OutputPin;
		}

		return Pin{};
	}

	bool CanCreateLink(ax::NodeEditor::PinId from, ax::NodeEditor::PinId to)
	{
		auto& fromPin = FindPin(from);
		auto& toPin = FindPin(to);

		if (IsPinConnected(from) || IsPinConnected(to))
			return false;

		// Cannot link same pin types
		if (fromPin.Kind == toPin.Kind)
			return false;

		return true;
	}

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

	u32 m_HeaderTextureID;
};