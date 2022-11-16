#pragma once
#include "Core.h"

#include <imgui_node_editor.h>

#include <vector>


enum class PinKind
{
	Input = 0,
	Output
};

enum class NodeType
{
	None = 0,
	Comment,
	Source,
	Scope
};

struct Node;

struct Pin
{
	std::string Name;
	ax::NodeEditor::PinId ID;
	PinKind Kind;
	Node* Node;
};

class Node
{
protected:
	std::string m_NodeName;
	Vector2f m_Position;
	Vector2f m_Size;

	ax::NodeEditor::NodeId m_NodeID;

	std::vector<Pin> m_InputPins;
	std::vector<Pin> m_OutputPins;

public:
	std::vector<Pin>& GetInputPins() { return m_InputPins; }
	std::vector<Pin>& GetOutputPins() { return m_OutputPins; }

	const std::string& GetName() const { return m_NodeName; }
	const Vector2f& GetPosition() const { return m_Position; }
	const Vector2f& GetSize() const { return m_Position; }
	ax::NodeEditor::NodeId GetNodeID() const { return m_NodeID; }

	void SetName(const std::string& name) { m_NodeName = name; }
	void SetPosition(const Vector2f& position) { m_Position = position; }
	void SetSize(const Vector2f& size) { m_Size = size; }

protected:
	void SetNodeID(ax::NodeEditor::NodeId nodeID) { m_NodeID = nodeID; }

public:
	virtual NodeType GetNodeType() const = 0;
	virtual void Render() = 0;
};