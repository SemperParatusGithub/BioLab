#include "LiveScript.h"

#include "UI/IconsMaterialDesign.h"

#include "NodeEditor/Nodes/Comment.h"
#include "NodeEditor/Nodes/Source.h"
#include "NodeEditor/Nodes/Scope.h"
#include "NodeEditor/Nodes/Filter.h"
#include "NodeEditor/Nodes/ProcessingNodes.h"


LiveScript::LiveScript()
{
	m_Channel1Node = CreateNode(ICON_MD_POST_ADD"  Channel 1", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 110.0f, 100.0f });
	m_Channel2Node = CreateNode(ICON_MD_POST_ADD"  Channel 2", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 110.0f, 100.0f });
	m_Channel3Node = CreateNode(ICON_MD_POST_ADD"  Channel 3", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 110.0f, 100.0f });
}
LiveScript::~LiveScript()
{
}

void LiveScript::Save(const std::string& filepath)
{
}
void LiveScript::Load(const std::string& filepath)
{
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

		m_Scopes.push_back((Scope*)node);
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