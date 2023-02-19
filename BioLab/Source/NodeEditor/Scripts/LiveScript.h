#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"
#include "NodeEditor/Nodes/Scope.h"

#include <imgui_node_editor.h>

#include <yaml-cpp/yaml.h>


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
	void SerializeNode(Node* node, YAML::Emitter& out);
	void SerializeLink(Link link, YAML::Emitter& out);

	Node* CreatePlainNode(Node::Type type, const std::string& name, ax::NodeEditor::NodeId id, const Vector2f& position, const Vector2f& size);

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

	bool IsPinConnected(ax::NodeEditor::PinId id)
	{
		for (auto& link : m_Links)
			if (link.StartPinID == id || link.EndPinID == id)
				return true;

		return false;
	}

	Node* FindNodeByID(ax::NodeEditor::NodeId id)
	{
		for (Node* node : m_Nodes)
		{
			if (node->id == id)
				return node;
		}

		return nullptr;
	}

	Node* FindNodeByName(const std::string& name)
	{
		for (Node* node : m_Nodes)
		{
			if (node->name == name)
				return node;
		}
		return nullptr;
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

	void LinkNodes(ax::NodeEditor::PinId startPinID, ax::NodeEditor::PinId endPinID)
	{
		auto& pin1 = FindPin(startPinID);
		auto& pin2 = FindPin(endPinID);

		Pin startPin = pin1.type == Pin::Type::Output ? pin1 : pin2;
		Pin endPin = pin1.type == Pin::Type::Output ? pin2 : pin1;

		startPin.node->nextLinkedNode = endPin.node;

		m_Links.push_back({ GetNextLinkID(), startPin.id, endPin.id });
	}

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
};