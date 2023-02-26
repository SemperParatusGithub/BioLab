#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"
#include "NodeEditor/Nodes/Scope.h"

#include <imgui_node_editor.h>

#include <yaml-cpp/yaml.h>


class NodeEditor;
class ScriptSerializer;

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

	void DeleteLink(ax::NodeEditor::LinkId id)
	{
		for (int i = 0; i < m_Links.size(); i++)
		{
			auto& link = m_Links[i];

			if (link.ID == id)
			{
				auto startPin = FindPin(link.StartPinID);
				startPin.node->nextLinkedNode = nullptr;
				m_Links.erase(m_Links.begin() + i);
			}
		}
	}
	void DeleteNode(ax::NodeEditor::NodeId id)
	{
		for (int i = 0; i < m_Nodes.size(); i++)
		{
			auto& node = m_Nodes[i];

			if (node->id == id)
			{
				if (node->inputPin.active)
				{
					auto& link = FindLink(node->inputPin.id);
					DeleteLink(link.ID);
				}
				if (node->outputPin.active)
				{
					auto& link = FindLink(node->outputPin.id);
					DeleteLink(link.ID);
				}

				delete m_Nodes[i];
				m_Nodes.erase(m_Nodes.begin() + i);
			}
		}
	}

private:
	void SerializeNode(Node* node, YAML::Emitter& out);
	void SerializeLink(Link link, YAML::Emitter& out);

private:
	Node* CreateNode(const std::string& name, Node::Type type, const Vector2f& position, const Vector2f& size);
	Node* CreatePlainNode(Node::Type type, const std::string& name, ax::NodeEditor::NodeId id, const Vector2f& position, const Vector2f& size);

	Node* FindNode(const std::string& name);
	Node* FindNode(ax::NodeEditor::NodeId id);
	Pin FindPin(ax::NodeEditor::PinId id);
	Link FindLink(ax::NodeEditor::PinId id)
	{
		for (auto& link : m_Links)
			if (link.StartPinID == id || link.EndPinID == id)
				return link;
		return Link();
	}

	bool IsPinConnected(ax::NodeEditor::PinId id);
	bool CanCreateLink(ax::NodeEditor::PinId from, ax::NodeEditor::PinId to);

	void LinkPins(ax::NodeEditor::PinId startPinID, ax::NodeEditor::PinId endPinID);

private:
	ax::NodeEditor::NodeId GetNextNodeID();
	ax::NodeEditor::LinkId GetNextLinkID();
	ax::NodeEditor::PinId GetNextPinID();

private:
	friend class NodeEditor;
	friend class ScriptSerializer;

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

	Node* m_InputSignalNode = nullptr;
	Node* m_OutputSignalNode = nullptr;
};