#pragma once
#include "Core.h"

#include <imgui_node_editor.h>
#include <vector>


struct Node;

struct Pin
{
	enum class Type
	{
		Input = 0,
		Output
	};

	std::string	name;
	Type type;
	ax::NodeEditor::PinId id;
	bool active = false;

	Node* node;
};

struct Link
{
	ax::NodeEditor::LinkId ID;

	ax::NodeEditor::PinId StartPinID;
	ax::NodeEditor::PinId EndPinID;

	ImColor Color = ImColor(255, 255, 255);
};

class Node
{
public:
	enum class Type
	{
		None = 0,
		Comment,
		Source,
		Scope,
		Filter
	};

	std::string	name;
	Type type = Type::None;
	ax::NodeEditor::NodeId id;

	Vector2f position;
	Vector2f size;

	Pin	inputPin;
	Pin	outputPin;

	Node* nextLinkedNode = nullptr;

public:
	virtual float ProcessSample(float newSample) { return 0.0f; };
	virtual void Render() {};
};