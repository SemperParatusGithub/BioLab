#pragma once
#include "Node.h"
#include "Nodes/Scope.h"


struct NodeEditorConfig
{
};

class NodeEditor
{
public:
	NodeEditor(const NodeEditorConfig& config = NodeEditorConfig{});
	~NodeEditor();

	void AddNewSample(const Vector4f& sample);

	void Render();
	void ShowDebugWindow();

	void Flow()
	{
		ax::NodeEditor::SetCurrentEditor(m_EditorContext);

		for (auto& link : m_Links)
			ax::NodeEditor::Flow(link.ID);

		ax::NodeEditor::SetCurrentEditor(nullptr);
	}

	Node* CreateNode(const std::string& name, Node::Type type, const Vector2f& position, const Vector2f& size);


	Node* FindNodeByID(ax::NodeEditor::NodeId id)
	{
		for (Node* node : m_Nodes)
		{
			if (node->id == id)
				return node;
		}

		return nullptr;
	}

	void SetupStyle();
	void SetupColors();

private:
	ax::NodeEditor::NodeId GetNextNodeID();
	ax::NodeEditor::LinkId GetNextLinkID();
	ax::NodeEditor::PinId GetNextPinID();

	void ProcessNodeWithSample(Node* node, float returnValue)
	{
		auto ret = node->ProcessSample(returnValue);
		if (node->nextLinkedNode != nullptr)
			ProcessNodeWithSample(node->nextLinkedNode, ret);
	}

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
			if (node->inputPin.id == id)
				return node->inputPin;
			if (node->outputPin.id == id)
				return node->outputPin;
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
		if (fromPin.type == toPin.type)
			return false;

		return true;
	}

	void HandleLinks();

	void DrawNode(Node* node);

public:
	std::vector<Scope*> GetScopes() { return m_Scopes; }


private:
	ax::NodeEditor::EditorContext* m_EditorContext;
	ImFont* m_Font;

	bool m_IsOpen = true;

	bool m_ShowDragDropTooltip = false;
	ax::NodeEditor::NodeId contextNodeId = 0;

	std::vector<Node*> m_Nodes;		// TODO: Delete me afterwards
	std::vector<Link> m_Links;

	int m_CurrentNodeID = 10000;
	int m_CurrentLinkID = 20000;
	int m_CurrentPinID = 30000;

	NodeEditorConfig m_Config;

	u32 m_HeaderTextureID;

	Node* m_Channel1Node;
	Node* m_Channel2Node;
	Node* m_Channel3Node;

	std::vector<Scope*> m_Scopes;
};