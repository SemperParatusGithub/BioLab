#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"
#include "NodeEditor/Nodes/Scope.h"

#include <imgui_node_editor.h>


class NodeEditor;

class LiveScript
{
public:
	LiveScript();
	~LiveScript();

	void Save(const std::string& filepath = "");
	void Load(const std::string& filepath = "");

	void Reset();

	ax::NodeEditor::NodeId GetNextNodeID();
	ax::NodeEditor::LinkId GetNextLinkID();
	ax::NodeEditor::PinId GetNextPinID();

	Node* CreateNode(const std::string& name, Node::Type type, const Vector2f& position, const Vector2f& size);

private:
	friend class NodeEditor;

private:
	std::string m_Filepath;

	std::vector<Node*> m_Nodes;		// TODO: Delete me afterwards
	std::vector<Link> m_Links;

	int m_CurrentNodeID = 10000;
	int m_CurrentLinkID = 20000;
	int m_CurrentPinID = 30000;

	Node* m_Channel1Node;
	Node* m_Channel2Node;
	Node* m_Channel3Node;

	std::vector<Scope*> m_Scopes;
};