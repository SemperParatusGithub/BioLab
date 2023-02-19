#include "LiveScript.h"

#include "UI/IconsMaterialDesign.h"

#include "NodeEditor/Nodes/Comment.h"
#include "NodeEditor/Nodes/Source.h"
#include "NodeEditor/Nodes/Scope.h"
#include "NodeEditor/Nodes/Filter.h"
#include "NodeEditor/Nodes/ProcessingNodes.h"

#include <fstream>


LiveScript::LiveScript()
{
	m_Channel1Node = CreateNode("Channel 1", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 110.0f, 100.0f });
	m_Channel2Node = CreateNode("Channel 2", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 110.0f, 100.0f });
	m_Channel3Node = CreateNode("Channel 3", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 110.0f, 100.0f });
}
LiveScript::~LiveScript()
{
}

void LiveScript::Save(const std::string& filepath)
{
	YAML::Emitter out;

	out << YAML::BeginMap;
	out << YAML::Key << "ScriptName" << YAML::Value << filepath;
	out << YAML::Key << "NodeCount" << YAML::Value << m_Nodes.size();
	out << YAML::Key << "NextNodeID" << YAML::Value << (intptr_t)GetNextNodeID().AsPointer();
	out << YAML::Key << "NextLinkID" << YAML::Value << (intptr_t)GetNextLinkID().AsPointer();
	out << YAML::Key << "NextPinID" << YAML::Value << (intptr_t)GetNextPinID().AsPointer();

	out << YAML::Key << "Nodes";
	out << YAML::BeginSeq;

	for (Node* node : m_Nodes)
	{
		SerializeNode(node, out);
	}

	out << YAML::EndSeq;

	out << YAML::Key << "Links";
	out << YAML::BeginSeq;

	for (Link& link : m_Links)
	{
		SerializeLink(link, out);
	}

	out << YAML::EndSeq;

	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();
	fout.flush();
	fout.close();
}
void LiveScript::Load(const std::string& filepath)
{
	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath);
	}
	catch (YAML::ParserException e)
	{
		std::cout << "Failed to load yaml file: " << e.what() << std::endl;
		__debugbreak();
	}

	m_Nodes.clear();
	m_Links.clear();

	std::string name = data["ScriptName"].as<std::string>();
	LOG_INFO("Decoding Script File: %s", name.c_str());

	m_CurrentNodeID = data["NextNodeID"].as<intptr_t>();
	m_CurrentLinkID = data["NextLinkID"].as<intptr_t>();
	m_CurrentPinID = data["NextPinID"].as<intptr_t>();

	auto nodes = data["Nodes"];
	auto links = data["Links"];

	for (auto& node : nodes)
	{
		auto nodeName = node["Name"].as<std::string>();
		auto nodeType = (Node::Type)node["Type"].as<int>();
		auto nodeID = ax::NodeEditor::NodeId(node["ID"].as<intptr_t>());

		auto position = node["Position"];
		auto size = node["Size"];

		auto inputPin = node["InputPin"];
		auto outputPin = node["OutputPin"];

		Vector2f nodePosition = { position["x"].as<float>(), position["y"].as<float>() };
		Vector2f nodeSize = { size["x"].as<float>(), size["y"].as<float>() };

		std::string inputPinName, outputPinName;
		ax::NodeEditor::PinId inputPinID, outputPinID;

		if (inputPin)
		{
			inputPinName = inputPin["Name"].as<std::string>();
			inputPinID = ax::NodeEditor::PinId(inputPin["ID"].as<intptr_t>());
		}
		if (outputPin)
		{
			outputPinName = outputPin["Name"].as<std::string>();
			outputPinID = ax::NodeEditor::PinId(outputPin["ID"].as<intptr_t>());
		}

		Node* newNode = CreatePlainNode(nodeType, nodeName, nodeID, nodePosition, nodeSize);

		switch (nodeType)
		{
			case Node::Type::Source:
				newNode->outputPin = Pin{ outputPinName, Pin::Type::Output, outputPinID, true, newNode };
				break;

			case Node::Type::Scope:
				newNode->inputPin = Pin{ inputPinName, Pin::Type::Input, inputPinID, true, newNode };
				break;

			case Node::Type::Filter:
			case Node::Type::Gain:
			case Node::Type::Absolute:
			case Node::Type::Offset:
				newNode->inputPin = Pin{ inputPinName, Pin::Type::Input, inputPinID, true, newNode };
				newNode->outputPin = Pin{ outputPinName, Pin::Type::Output, outputPinID, true, newNode };
				break;	
		}
	}

	for (auto& link : links)
	{
		auto& linkID = ax::NodeEditor::LinkId(link["ID"].as<intptr_t>());
		auto& startPinID = ax::NodeEditor::PinId(link["StartPinID"].as<intptr_t>());
		auto& endPinID = ax::NodeEditor::PinId(link["EndPinID"].as<intptr_t>());

		auto& startPin = FindPin(startPinID);
		auto& endPin = FindPin(endPinID);

		startPin.node->nextLinkedNode = endPin.node;

		m_Links.push_back({ linkID, startPinID, endPinID });
	}

	m_Channel1Node = FindNodeByName("Channel 1");
	m_Channel2Node = FindNodeByName("Channel 2");
	m_Channel3Node = FindNodeByName("Channel 3");
}

void LiveScript::Reset()
{
}


ax::NodeEditor::NodeId LiveScript::GetNextNodeID()
{
	m_CurrentNodeID++;
	return ax::NodeEditor::NodeId(m_CurrentNodeID);
}
ax::NodeEditor::LinkId LiveScript::GetNextLinkID()
{
	m_CurrentLinkID++;
	return ax::NodeEditor::LinkId(m_CurrentLinkID);
}
ax::NodeEditor::PinId LiveScript::GetNextPinID()
{
	m_CurrentPinID++;
	return ax::NodeEditor::PinId(m_CurrentPinID);
}


Node* LiveScript::CreateNode(const std::string& name, Node::Type type, const Vector2f& position, const Vector2f& size)
{
	Node* node = nullptr;

	switch (type)
	{
	case Node::Type::Comment:
	{
		m_Nodes.emplace_back(new Comment(GetNextNodeID(), name, position, size));
		node = m_Nodes.back();
		break;
	}
	case Node::Type::Source:
	{
		m_Nodes.emplace_back(new Source(GetNextNodeID(), name, position, size));
		node = m_Nodes.back();
		node->outputPin = Pin{ "Output1", Pin::Type::Output, GetNextPinID(), true, node };
		break;
	}
	case Node::Type::Scope:
	{
		m_Nodes.emplace_back(new Scope(GetNextNodeID(), name, position, size));
		node = m_Nodes.back();
		node->inputPin = Pin{ "Input1", Pin::Type::Input, GetNextPinID(), true, node };
		break;
	}
	case Node::Type::Filter:
	{
		m_Nodes.emplace_back(new Filter(GetNextNodeID(), name, position, size));
		node = m_Nodes.back();
		node->inputPin = Pin{ "Input1", Pin::Type::Input, GetNextPinID(), true, node };
		node->outputPin = Pin{ "Output1", Pin::Type::Output, GetNextPinID(), true, node };
		break;
	}
	case Node::Type::Gain:
	{
		m_Nodes.emplace_back(new Gain(GetNextNodeID(), name, position, size));
		node = m_Nodes.back();
		node->inputPin = Pin{ "Input1", Pin::Type::Input, GetNextPinID(), true, node };
		node->outputPin = Pin{ "Output1", Pin::Type::Output, GetNextPinID(), true, node };
		break;
	}
	case Node::Type::Offset:
	{
		m_Nodes.emplace_back(new Offset(GetNextNodeID(), name, position, size));
		node = m_Nodes.back();
		node->inputPin = Pin{ "Input1", Pin::Type::Input, GetNextPinID(), true, node };
		node->outputPin = Pin{ "Output1", Pin::Type::Output, GetNextPinID(), true, node };
		break;
	}
	case Node::Type::Absolute:
	{
		m_Nodes.emplace_back(new Absolute(GetNextNodeID(), name, position, size));
		node = m_Nodes.back();
		node->inputPin = Pin{ "Input1", Pin::Type::Input, GetNextPinID(), true, node };
		node->outputPin = Pin{ "Output1", Pin::Type::Output, GetNextPinID(), true, node };
		break;
	}
	}

	return node;
}

void LiveScript::SerializeNode(Node* node, YAML::Emitter& out)
{
	out << YAML::BeginMap;  // Node

	out << YAML::Key << "Name" << YAML::Value << node->name;
	out << YAML::Key << "Type" << YAML::Value << (int)node->type;
	out << YAML::Key << "ID" << YAML::Value << (intptr_t)node->id.AsPointer();

	if (node->inputPin.active)
	{
		out << YAML::Key << "InputPin";
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << node->inputPin.name;
		out << YAML::Key << "ID" << YAML::Value << (intptr_t)node->inputPin.id.AsPointer();
		out << YAML::EndMap;
	}	
	
	if (node->outputPin.active)
	{
		out << YAML::Key << "OutputPin";
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << node->outputPin.name;
		out << YAML::Key << "ID" << YAML::Value << (intptr_t)node->outputPin.id.AsPointer();
		out << YAML::EndMap;
	}

	out << YAML::Key << "Position";
	out << YAML::BeginMap;
	out << YAML::Key << "x" << YAML::Value << node->position.x;
	out << YAML::Key << "y" << YAML::Value << node->position.y;
	out << YAML::EndMap;

	out << YAML::Key << "Size";
	out << YAML::BeginMap;
	out << YAML::Key << "x" << YAML::Value << node->size.x;
	out << YAML::Key << "y" << YAML::Value << node->size.y;
	out << YAML::EndMap;


	out << YAML::EndMap;    // Node
}

void LiveScript::SerializeLink(Link link, YAML::Emitter& out)
{
	out << YAML::BeginMap;

	out << YAML::Key << "ID" << YAML::Value << (intptr_t)link.ID.AsPointer();
	out << YAML::Key << "StartPinID" << YAML::Value << (intptr_t)link.StartPinID.AsPointer();
	out << YAML::Key << "EndPinID" << YAML::Value << (intptr_t)link.EndPinID.AsPointer();

	out << YAML::EndMap;
}

Node* LiveScript::CreatePlainNode(Node::Type type, const std::string& name, ax::NodeEditor::NodeId id, const Vector2f& position, const Vector2f& size)
{
	switch (type)
	{
	case Node::Type::Comment:  m_Nodes.emplace_back(new Comment(id, name, position, size));  break;
	case Node::Type::Source:   m_Nodes.emplace_back(new Source(id, name, position, size));   break;
	case Node::Type::Scope:    m_Nodes.emplace_back(new Scope(id, name, position, size));    break;

	case Node::Type::Filter:   m_Nodes.emplace_back(new Filter(id, name, position, size));   break;
	case Node::Type::Gain:     m_Nodes.emplace_back(new Gain(id, name, position, size));     break;
	case Node::Type::Offset:   m_Nodes.emplace_back(new Offset(id, name, position, size));   break;
	case Node::Type::Absolute: m_Nodes.emplace_back(new Absolute(id, name, position, size)); break;
	}
	return m_Nodes.back();
}