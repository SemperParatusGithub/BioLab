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
	bool Active = false;
};

class Node
{
public:
	std::string Name;
	ax::NodeEditor::NodeId ID;

	Vector2f Position;
	Vector2f Size;

	Pin InputPin;
	Pin OutputPin;

public:
	virtual NodeType GetNodeType() const = 0;
	virtual void Render() = 0;
};