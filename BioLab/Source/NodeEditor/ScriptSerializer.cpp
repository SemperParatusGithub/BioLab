#include "ScriptSerializer.h"

#include <fstream>


void ScriptSerializer::SerializeScript(Script& script, const std::string& filepath)
{
	LOG_INFO("Serializing Script: %s", filepath.c_str());

	if (filepath.empty())
	{
		LOG_ERROR("Failed to serialize Script: no filepath specified");
		return;
	}

	YAML::Emitter out;

	out << YAML::BeginMap;
	out << YAML::Key << "ScriptName" << YAML::Value << filepath;
	out << YAML::Key << "ScriptType" << YAML::Value << (int)script.m_Type;
	out << YAML::Key << "NodeCount" << YAML::Value << script.m_Nodes.size();
	out << YAML::Key << "NextNodeID" << YAML::Value << (intptr_t)script.GetNextNodeID().AsPointer();
	out << YAML::Key << "NextLinkID" << YAML::Value << (intptr_t)script.GetNextLinkID().AsPointer();
	out << YAML::Key << "NextPinID" << YAML::Value << (intptr_t)script.GetNextPinID().AsPointer();

	// Serialize Nodes
	{
		out << YAML::Key << "Nodes";
		out << YAML::BeginSeq;

		for (Node* node : script.m_Nodes)
			SerializeNode(node, out);

		out << YAML::EndSeq;
	}

	// Serialize Links
	{
		out << YAML::Key << "Links";
		out << YAML::BeginSeq;

		for (Link& link : script.m_Links)
			SerializeLink(link, out);

		out << YAML::EndSeq;
	}

	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();
	fout.flush();
	fout.close();
}
Script ScriptSerializer::DeserializeScript(const std::string& filepath)
{
	LOG_INFO("Deserializing Script: %s", filepath.c_str());

	if (filepath.empty())
	{
		LOG_ERROR("Failed to deserialize Script: no filepath specified");
		return Script();
	}

	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath);
	}
	catch (YAML::ParserException e)
	{
		LOG_ERROR("Failed to load yaml file: ", e.what());
		return Script();
	}

	Script newScript;

	newScript.m_Name = data["ScriptName"].as<std::string>();
	newScript.m_Type = (Script::Type)data["ScriptType"].as<int>();

	newScript.m_CurrentNodeID = data["NextNodeID"].as<intptr_t>();
	newScript.m_CurrentLinkID = data["NextLinkID"].as<intptr_t>();
	newScript.m_CurrentPinID = data["NextPinID"].as<intptr_t>();

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

		Node* newNode = newScript.CreatePlainNode(nodeType, nodeName, nodeID, nodePosition, nodeSize);

		if (inputPin)
		{
			inputPinName = inputPin["Name"].as<std::string>();
			inputPinID = ax::NodeEditor::PinId(inputPin["ID"].as<intptr_t>());
			newNode->inputPin = Pin{ inputPinName, Pin::Type::Input, inputPinID, true, newNode };
		}
		if (outputPin)
		{
			outputPinName = outputPin["Name"].as<std::string>();
			outputPinID = ax::NodeEditor::PinId(outputPin["ID"].as<intptr_t>());
			newNode->outputPin = Pin{ outputPinName, Pin::Type::Output, outputPinID, true, newNode };
		}
	}

	for (auto& link : links)
	{
		auto& linkID = ax::NodeEditor::LinkId(link["ID"].as<intptr_t>());
		auto& startPinID = ax::NodeEditor::PinId(link["StartPinID"].as<intptr_t>());
		auto& endPinID = ax::NodeEditor::PinId(link["EndPinID"].as<intptr_t>());

		auto& startPin = newScript.FindPin(startPinID);
		auto& endPin = newScript.FindPin(endPinID);

		startPin.node->nextLinkedNode = endPin.node;

		newScript.m_Links.push_back({ linkID, startPinID, endPinID });
	}

	newScript.m_Channel1Node = newScript.FindNode("Channel 1");
	newScript.m_Channel2Node = newScript.FindNode("Channel 2");
	newScript.m_Channel3Node = newScript.FindNode("Channel 3");	
	
	newScript.m_InputSignalNode = newScript.FindNode("Input Signal");
	newScript.m_OutputSignalNode = newScript.FindNode("Output Signal");

	return newScript;
}

Script ScriptSerializer::CreateNewLiveScript()
{
	Script newScript;
	newScript.m_Type = Script::Type::LiveScript;
	newScript.m_Channel1Node = newScript.CreateNode("Channel 1", Node::Type::Source, Vector2f{ 0.0f, 0.0f }, Vector2f{ 100.0f, 100.0f });
	newScript.m_Channel2Node = newScript.CreateNode("Channel 2", Node::Type::Source, Vector2f{ 0.0f, 0.0f }, Vector2f{ 100.0f, 100.0f });
	newScript.m_Channel3Node = newScript.CreateNode("Channel 3", Node::Type::Source, Vector2f{ 0.0f, 0.0f }, Vector2f{ 100.0f, 100.0f });

	return newScript;
}
Script ScriptSerializer::CreateNewPostProcessScript()
{
	Script newScript;
	newScript.m_Type = Script::Type::PostProcessScript;
	newScript.m_InputSignalNode = newScript.CreateNode("Input Signal", Node::Type::InputSignal, Vector2f{ 0.0f, 0.0f }, Vector2f{ 200.0f, 200.0f });
	newScript.m_OutputSignalNode = newScript.CreateNode("Output Signal", Node::Type::OutputSignal, Vector2f{ 0.0f, 0.0f }, Vector2f{ 200.0f, 200.0f });

	return newScript;
}

void ScriptSerializer::SerializeNode(Node* node, YAML::Emitter& out)
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
void ScriptSerializer::SerializeLink(Link link, YAML::Emitter& out)
{
	out << YAML::BeginMap;

	out << YAML::Key << "ID" << YAML::Value << (intptr_t)link.ID.AsPointer();
	out << YAML::Key << "StartPinID" << YAML::Value << (intptr_t)link.StartPinID.AsPointer();
	out << YAML::Key << "EndPinID" << YAML::Value << (intptr_t)link.EndPinID.AsPointer();

	out << YAML::EndMap;
}