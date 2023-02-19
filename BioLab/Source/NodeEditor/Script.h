#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"
#include "NodeEditor/Nodes/Scope.h"

#include <imgui_node_editor.h>

#include <yaml-cpp/yaml.h>


class NodeEditor;

class Script
{
public:
	enum class Type
	{
		None = 0,
		LiveScript,
		PostProcessScript
	};

public:
	Script();
	Script(const std::string& filepath);
	~Script();

	void Clear();

	Type GetScriptType() const { return m_Type; }
	const std::string& GetName() const { return m_Name; }

	void Serialize(const std::string& filepath);
	void Deserialize(const std::string& filepath);

	void SerializeNode(Node* node, YAML::Emitter& out);
	void SerializeLink(Link link, YAML::Emitter& out);

private:
	Node* CreateNode(const std::string& name, Node::Type type, const Vector2f& position, const Vector2f& size);
	Node* CreatePlainNode(Node::Type type, const std::string& name, ax::NodeEditor::NodeId id, const Vector2f& position, const Vector2f& size);

	Node* FindNode(const std::string& name);
	Node* FindNode(ax::NodeEditor::NodeId id);
	Pin FindPin(ax::NodeEditor::PinId id);

	bool IsPinConnected(ax::NodeEditor::PinId id);
	bool CanCreateLink(ax::NodeEditor::PinId from, ax::NodeEditor::PinId to);

	void LinkPins(ax::NodeEditor::PinId startPinID, ax::NodeEditor::PinId endPinID);

private:
	ax::NodeEditor::NodeId GetNextNodeID();
	ax::NodeEditor::LinkId GetNextLinkID();
	ax::NodeEditor::PinId GetNextPinID();

private:
	friend class NodeEditor;

	Type m_Type = Type::None;
	std::string m_Name = "Unknown";

	std::vector<Node*> m_Nodes;
	std::vector<Link> m_Links;

	int m_CurrentNodeID = 10000;
	int m_CurrentLinkID = 20000;
	int m_CurrentPinID = 30000;

	Node* m_Channel1Node = nullptr;
	Node* m_Channel2Node = nullptr;
	Node* m_Channel3Node = nullptr;
};