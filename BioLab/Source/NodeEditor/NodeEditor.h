#pragma once
#include "Node.h"
#include "Nodes/Scope.h"

#include "Scripts/LiveScript.h"


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

	void Render();
	void ShowDebugWindow();

	void Flow();

	void SetupStyle();
	void SetupColors();

	void SaveLiveScript(const std::string& filepath);
	void LoadLiveScript(const std::string& filepath);

	Node* FindNodeByID(ax::NodeEditor::NodeId id)
	{
		return m_LiveScript.FindNodeByID(id);
	}

private:
	void ProcessNodeWithSample(Node* node, float returnValue)
	{
		auto ret = node->ProcessSample(returnValue);
		if (node->nextLinkedNode != nullptr)
			ProcessNodeWithSample(node->nextLinkedNode, ret);
	}

	void HandleLinks();

	void DrawNode(Node* node);

public:
	std::vector<Scope*> GetScopes();


private:
	ax::NodeEditor::EditorContext* m_EditorContext;
	ImFont* m_Font;

	bool m_IsOpen = true;

	bool m_ShowDragDropTooltip = false;
	ax::NodeEditor::NodeId contextNodeId = 0;

	LiveScript m_LiveScript;

	NodeEditorConfig m_Config;
};