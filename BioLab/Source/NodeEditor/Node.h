#pragma once
#include "Core.h"

#include <imgui_node_editor.h>
#include <vector>

#include "SignalProcessing/Signal.h"

#include "UI/IconsMaterialDesign.h"


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

	Node* node = nullptr;
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

		// Live processing nodes:
		Source,
		Scope,

		// Post processing nodes
		InputSignal,
		OutputSignal,
		FourierTransform,
		Normalize,
		RemoveDC,

		Filter,
		Gain,
		Offset,
		Absolute,
		Average
	};

	std::string	name;
	Type type = Type::None;
	ax::NodeEditor::NodeId id;

	Vector2f position = { 0.0f, 0.0f };
	Vector2f size = { 200.0f, 200.0f };

	Pin	inputPin;
	Pin	outputPin;

	Node* nextLinkedNode = nullptr;

public:
	virtual float ProcessSample(float newSample)		{ return newSample; };
	virtual Signal ProcessSignal(const Signal& signal)	{ return signal; };
	virtual void Render()								{};
};