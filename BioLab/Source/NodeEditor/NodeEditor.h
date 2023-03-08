#pragma once
#include "Node.h"
#include "Nodes/Scope.h"

#include "Scripts/LiveScript.h"

#include "Script.h"
#include "ScriptSerializer.h"

#include "SignalProcessing/Signal.h"

struct NodeEditorConfig
{
};

class NodeEditor
{
public:
	NodeEditor(const NodeEditorConfig& config = NodeEditorConfig{});
	~NodeEditor();

	void AddNewSample(const Vector4f& sample);
	void ClearScopeBuffers();

	void ExecutePostProcessScript();

	void Render();
	void ShowDebugWindow();

	void Flow();

	void SetupStyle();
	void SetLightColorTheme();
	void SetDarkColorTheme();

	void SaveLiveScript(const std::string& filepath);
	void LoadLiveScript(const std::string& filepath);

	void CreateLiveScript();
	void CreatePostProcessScript();

	Node* FindNodeByID(ax::NodeEditor::NodeId id) { return m_ActiveScript.FindNode(id); }
	const Script& GetActiveScript() const { return m_ActiveScript; }

	void ProhibitDeletions() { m_DeletionProhibited = true; }
	void AllowDeletions() { m_DeletionProhibited = false; }

private:
	void ProcessNodeWithSample(Node* node, float returnValue)
	{
		float result = node->ProcessSample(returnValue);
		if (node->nextLinkedNode != nullptr)
			ProcessNodeWithSample(node->nextLinkedNode, result);
	}
	void ProcessNodeWithSignal(Node* node, Signal signal)
	{
		Signal result = node->ProcessSignal(signal);

		if (node->nextLinkedNode != nullptr)
			ProcessNodeWithSignal(node->nextLinkedNode, result);
	}

	void UpdateNodePositions();
	void NavigateToContent();
	void HandleLinks();

	void DrawNode(Node* node);

	bool CanNodeBeModified(ax::NodeEditor::NodeId id)
	{
		Node* node = FindNodeByID(id);
		if (node == nullptr)
			return false;

		return node->name != "Channel 1" && node->name != "Channel 2" && node->name != "Channel 3" &&
			node->name != "Input Signal" && node->name != "Output Signal";
	}

private:
	NodeEditorConfig m_Config;
	ax::NodeEditor::EditorContext* m_EditorContext;

	bool m_IsOpen = false;
	bool m_DeletionProhibited = false;

	bool m_ShowDragDropTooltip = false;
	ax::NodeEditor::NodeId contextNodeId = 0;

	Script m_ActiveScript;
	ScriptSerializer m_ScriptSerializer;
};