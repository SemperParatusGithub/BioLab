#include "NodeEditor.h"

#include "Nodes/Comment.h"
#include "Nodes/Filter.h"
#include "Nodes/LiveNodes.h"
#include "Nodes/OfflineNodes.h"
#include "Nodes/ProcessingNodes.h"

#include "UI/UICore.h"
#include "UI/IconsMaterialDesign.h"

#include "Application.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <implot.h>

#include <stb_image.h>
#include <GLFW/glfw3.h>


NodeEditor::NodeEditor(const NodeEditorConfig& config)
{
	m_Config = config;

	ax::NodeEditor::Config internalConfig;
	internalConfig.SettingsFile = nullptr;
	m_EditorContext = ax::NodeEditor::CreateEditor(&internalConfig);

	ax::NodeEditor::SetCurrentEditor(m_EditorContext);
	ax::NodeEditor::GetStyle() = ax::NodeEditor::Style();
	ax::NodeEditor::GetStyle().FlowDuration = 1.0f;
	ax::NodeEditor::GetStyle().LinkStrength = 350.0f;
	ax::NodeEditor::SetCurrentEditor(nullptr);
}
NodeEditor::~NodeEditor()
{
}

void NodeEditor::AddNewSample(const Vector4f& sample)
{
	Flow();

	if (m_ActiveScript.m_Type == Script::Type::LiveScript)
	{
		ProcessNodeWithSample(m_ActiveScript.m_Channel1Node, sample.y);
		ProcessNodeWithSample(m_ActiveScript.m_Channel2Node, sample.z);
		ProcessNodeWithSample(m_ActiveScript.m_Channel3Node, sample.w);
	}
	else {
		//LOG_INFO("Warning: No LiveScript selected");
	}
}
void NodeEditor::ClearScopeBuffers()
{
	for (Node* node : m_ActiveScript.m_Nodes)
	{
		if (node->type == Node::Type::Scope)
		{
			Scope* scopeNode = reinterpret_cast<Scope*>(node);
			scopeNode->ClearBuffer();
		}
	}
}

void NodeEditor::ExecutePostProcessScript()
{
	if (m_ActiveScript.m_Type != Script::Type::PostProcessScript)
	{
		LOG_ERROR("Failed to execute Script: m_ActiveScript.m_Type != Script::Type::PostProcessScript");
		return;
	}

	Flow();

	Node* inputSignalNode = m_ActiveScript.m_InputSignalNode;
	Node* outputSignalNode = m_ActiveScript.m_OutputSignalNode;

	int inputSignalID = ((InputSignal*)inputSignalNode)->GetSignalID();

	LOG_INFO("Executing PostProcessScript");
	LOG_INFO("\tInputSignalID: %d", inputSignalID);

	// 1. retrieve Signal from Application
	Application* instance = Application::Instance();
	Signal* inputSignal = instance->GetSignalByID(inputSignalID);

	if (inputSignal == nullptr)
	{
		LOG_ERROR("No Input Signal defined");
		return;
	}

	// 2. Process nodes recursively
	ProcessNodeWithSignal(m_ActiveScript.m_InputSignalNode, *inputSignal);
	OutputSignal outputSignal = (*(OutputSignal*)outputSignalNode);

	// 3. emplace output to Application
	instance->m_LoadedSignals.push_back(outputSignal.GetSignal());
}

void NodeEditor::Render()
{
	//if (m_ActiveScript.m_Type == Script::Type::None)
	//	return;

	bool showDragTooltip = false;

	if (!m_IsOpen)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(ICON_MD_INSERT_CHART" Node Editor", &m_IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		ax::NodeEditor::SetCurrentEditor(m_EditorContext);
		ImGui::PushFont(UICore::GetFont(Font::Default));
		ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
		ax::NodeEditor::Begin("Node Editor", ImGui::GetContentRegionAvail());
		for (auto* node : m_ActiveScript.m_Nodes)
		{
			// comments are handled specially
			if (node->type == Node::Type::Comment)
			{
				node->Render();
				continue;
			}

			DrawNode(node);
		}

		HandleLinks();

		// Render Popups
		{
			ax::NodeEditor::Suspend();

			if (ax::NodeEditor::ShowBackgroundContextMenu()) 
			{
				ImGui::OpenPopup("Create New Node");
			}
			else if (ax::NodeEditor::ShowNodeContextMenu(&contextNodeId))
			{
				if(CanNodeBeModified(contextNodeId))
					ImGui::OpenPopup("Node Context");
			}
			else {
				ImGui::CloseCurrentPopup();
			}


			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

			if (ImGui::BeginPopup("Create New Node"))
			{
				if (ImGui::MenuItem("Comment"))
					m_ActiveScript.CreateNode("Comment", Node::Type::Comment, { 100.0f, 100.0f }, { 500.0f, 400.0f });
				if (ImGui::MenuItem("Scope"))
					m_ActiveScript.CreateNode("Scope", Node::Type::Scope, { 100.0f, 100.0f }, { 150.0f, 100.0f });
				if (ImGui::MenuItem("Filter"))
					m_ActiveScript.CreateNode("Filter", Node::Type::Filter, { 500.0f, 500.0f }, { 350.0f, 400.0f });
				if (ImGui::MenuItem("Gain"))
					m_ActiveScript.CreateNode("Gain", Node::Type::Gain, { 500.0f, 500.0f }, { 200.0f, 100.0f });
				if (ImGui::MenuItem("Offset"))
					m_ActiveScript.CreateNode("Offset", Node::Type::Offset, { 500.0f, 500.0f }, { 200.0f, 100.0f });
				if (ImGui::MenuItem("Absolute"))
					m_ActiveScript.CreateNode("Absolute", Node::Type::Absolute, { 500.0f, 500.0f }, { 200.0f, 100.0f });		
				if (ImGui::MenuItem("Average"))
					m_ActiveScript.CreateNode("Average", Node::Type::Average, { 500.0f, 500.0f }, { 200.0f, 100.0f });	
				
				if (m_ActiveScript.m_Type == Script::Type::PostProcessScript)
					if (ImGui::MenuItem("DFT"))
						m_ActiveScript.CreateNode("DFT", Node::Type::FourierTransform, { 500.0f, 500.0f }, { 200.0f, 100.0f });

				ImGui::EndPopup();
			}
			if (ImGui::BeginPopup("Node Context"))
			{
				Node* node = m_ActiveScript.FindNode(contextNodeId);
				char buf[32];
				strcpy(buf, node->name.c_str());
				ImGui::SetNextItemWidth(200.0f);

				ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;
				if (node->name == "Channel 1" || node->name == "Channel 2" || node->name == "Channel 3")
					flags |= ImGuiInputTextFlags_ReadOnly;

				if (ImGui::InputText("##Name", buf, 32, flags))
					node->name = std::string(buf);

				if (ImGui::IsKeyPressed(ImGuiKey_Enter))
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
			}
			if (m_ShowDragDropTooltip)
			{
				ImGui::SetNextWindowPos(ImGui::GetMousePos());
				ImGui::BeginTooltip();
				ImGui::Text("Drag to plot window");
				ImGui::EndTooltip();
			}

			ImGui::PopStyleVar();
			ax::NodeEditor::Resume();
		}

		ax::NodeEditor::End();
		ax::NodeEditor::PopStyleVar();
		ImGui::PopFont();

		ax::NodeEditor::SetCurrentEditor(nullptr);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}
void NodeEditor::ShowDebugWindow()
{
	ImGui::Begin("Node Editor Debug");
	ImGui::Text("m_CurrentNodeID: %d", m_ActiveScript.m_CurrentNodeID);
	ImGui::Text("m_CurrentLinkID: %d", m_ActiveScript.m_CurrentLinkID);
	ImGui::Text("m_CurrentPinID: %d", m_ActiveScript.m_CurrentPinID);

	if (ImGui::CollapsingHeader("Nodes", ImGuiTreeNodeFlags_DefaultOpen))
		for (auto* node : m_ActiveScript.m_Nodes)
		{
			ImGui::Text(node->name.c_str());
			ImGui::Text("ID: %d", node->id);
			ImGui::Text("Position: %.2f, %.2f", node->position.x, node->position.y);
			ImGui::Text("Size: %.2f, %.2f", node->size.x, node->size.y);
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
		}

	if (ImGui::CollapsingHeader("Links", ImGuiTreeNodeFlags_DefaultOpen))
		for (auto& link : m_ActiveScript.m_Links)
			ImGui::Text("Link %d   from %d to %d", link.ID.AsPointer(), link.StartPinID.AsPointer(), link.EndPinID.AsPointer());

	if (ImGui::Button("Clear Script"))
		m_ActiveScript.Clear();

	if (ImGui::Button("Navigate to content"))
		NavigateToContent();

	ImGui::End();
}

void NodeEditor::Flow()
{
	ax::NodeEditor::SetCurrentEditor(m_EditorContext);

	for (auto& link : m_ActiveScript.m_Links)
		ax::NodeEditor::Flow(link.ID);

	ax::NodeEditor::SetCurrentEditor(nullptr);
}

void NodeEditor::SetupStyle()
{
	ax::NodeEditor::SetCurrentEditor(m_EditorContext);

	ax::NodeEditor::Style& nodeStyle = ax::NodeEditor::GetStyle();
	//nodeStyle.NodeRounding = 0.0f;
	nodeStyle.PinRounding = 0.0f;
	nodeStyle.SelectedNodeBorderWidth = 4.5f;
	nodeStyle.HoveredNodeBorderWidth = 4.5f;
	nodeStyle.NodeBorderWidth = 2.5f;

	ax::NodeEditor::SetCurrentEditor(nullptr);
}
void NodeEditor::SetLightColorTheme()
{
	ax::NodeEditor::SetCurrentEditor(m_EditorContext);

	ax::NodeEditor::Style& nodeStyle = ax::NodeEditor::GetStyle();
	ImVec4* nodeColors = nodeStyle.Colors;

	nodeColors[ax::NodeEditor::StyleColor_Bg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	nodeColors[ax::NodeEditor::StyleColor_Grid] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	nodeColors[ax::NodeEditor::StyleColor_NodeBg] = ImVec4(0.9f, 0.9f, 0.9f, 0.7f);
	nodeColors[ax::NodeEditor::StyleColor_NodeBorder] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
	// nodeColors[ax::NodeEditor::StyleColor_HovNodeBorder]
	// nodeColors[ax::NodeEditor::StyleColor_SelNodeBorder]
	// nodeColors[ax::NodeEditor::StyleColor_NodeSelRect]
	// nodeColors[ax::NodeEditor::StyleColor_HovLinkBorder]
	// nodeColors[ax::NodeEditor::StyleColor_SelLinkBorder]
	// nodeColors[ax::NodeEditor::StyleColor_HighlightLinkBorder]
	// nodeColors[ax::NodeEditor::StyleColor_LinkSelRect]
	// nodeColors[ax::NodeEditor::StyleColor_LinkSelRectBorder]
	// nodeColors[ax::NodeEditor::StyleColor_PinRect]
	// nodeColors[ax::NodeEditor::StyleColor_PinRectBorder]
	// nodeColors[ax::NodeEditor::StyleColor_Flow]
	// nodeColors[ax::NodeEditor::StyleColor_FlowMarker]
	nodeColors[ax::NodeEditor::StyleColor_GroupBg] = ImVec4(0.95f, 0.95f, 0.95f, 0.7f);
	nodeColors[ax::NodeEditor::StyleColor_GroupBorder] = ImVec4(0.95f, 0.95f, 0.95f, 0.7f);;

	ax::NodeEditor::SetCurrentEditor(nullptr);
}
void NodeEditor::SetDarkColorTheme()
{
	ax::NodeEditor::SetCurrentEditor(m_EditorContext);

	ax::NodeEditor::Style& nodeStyle = ax::NodeEditor::GetStyle();
	ImVec4* nodeColors = ax::NodeEditor::Style().Colors;

	ax::NodeEditor::SetCurrentEditor(nullptr);
}

void NodeEditor::SaveLiveScript(const std::string& filepath)
{
	ax::NodeEditor::SetCurrentEditor(m_EditorContext);

	for (Node* node : m_ActiveScript.m_Nodes)
	{
		ImVec2 pos = ax::NodeEditor::GetNodePosition(node->id);
		// ImVec2 size = ax::NodeEditor::GetNodeSize(node->id);

		node->position = Vector2f{ pos.x, pos.y };
		// node->size = Vector2f{ size.x, size.y };
	}

	ax::NodeEditor::SetCurrentEditor(nullptr);

	m_ScriptSerializer.SerializeScript(m_ActiveScript, filepath);
}
void NodeEditor::LoadLiveScript(const std::string& filepath)
{
	m_IsOpen = true;

	m_ActiveScript = m_ScriptSerializer.DeserializeScript(filepath);
	UpdateNodePositions();
	NavigateToContent();
}

void NodeEditor::CreateLiveScript()
{
	m_ActiveScript = m_ScriptSerializer.CreateNewLiveScript();
	m_IsOpen = true;
	UpdateNodePositions();
	NavigateToContent();
}
void NodeEditor::CreatePostProcessScript()
{
	m_ActiveScript = m_ScriptSerializer.CreateNewPostProcessScript();
	m_IsOpen = true;
	UpdateNodePositions();
}

void NodeEditor::UpdateNodePositions()
{
	ax::NodeEditor::SetCurrentEditor(m_EditorContext);

	for (Node* node : m_ActiveScript.m_Nodes)
		ax::NodeEditor::SetNodePosition(node->id, { node->position.x, node->position.y });

	ax::NodeEditor::NavigateToContent();
	ax::NodeEditor::SetCurrentEditor(nullptr);
}
void NodeEditor::NavigateToContent()
{
	ax::NodeEditor::SetCurrentEditor(m_EditorContext);
	ax::NodeEditor::NavigateToContent();
	ax::NodeEditor::SetCurrentEditor(nullptr);
}

void NodeEditor::HandleLinks()
{
	// taken from examples

	// Submit Links
	for (auto& link : m_ActiveScript.m_Links)
		ax::NodeEditor::Link(link.ID, link.StartPinID, link.EndPinID, ImVec4(0.26f, 0.59f, 0.98f, 0.75f), 5.0f);

	// Handle creation action, returns true if editor want to create new object (node or link)
	if (ax::NodeEditor::BeginCreate(ImVec4(0.26f, 0.59f, 0.98f, 0.75f), 6.0f))
	{
		ax::NodeEditor::PinId inputPinId, outputPinId;
		if (ax::NodeEditor::QueryNewLink(&inputPinId, &outputPinId))
		{
			// QueryNewLink returns true if editor want to create new link between pins.
			//
			// Link can be created only for two valid pins, it is up to you to
			// validate if connection make sense. Editor is happy to make any.
			//
			// Link always goes from input to output. User may choose to drag
			// link from output pin or input pin. This determine which pin ids
			// are valid and which are not:
			//   * input valid, output invalid - user started to drag new ling from input pin
			//   * input invalid, output valid - user started to drag new ling from output pin
			//   * input valid, output valid   - user dragged link over other pin, can be validated

			if (m_ActiveScript.CanCreateLink(inputPinId, outputPinId)) // both are valid, let's accept link
			{
				// ax::NodeEditor::AcceptNewItem() return true when user release mouse button.
				if (ax::NodeEditor::AcceptNewItem())
				{
					// Since we accepted new link, lets add one to our list of links.

					m_ActiveScript.LinkPins(inputPinId, outputPinId);
				}

				// You may choose to reject connection between these nodes
				// by calling ax::NodeEditor::RejectNewItem(). This will allow editor to give
				// visual feedback by changing link thickness and color.
			}
		}
	}
	ax::NodeEditor::EndCreate(); // Wraps up object creation action handling.

	// Handle deletion action
	if (m_DeletionProhibited)
		return;
	if (ax::NodeEditor::BeginDelete())
	{
		// There may be many links marked for deletion, let's loop over them.
		ax::NodeEditor::LinkId deletedLinkId;
		while (ax::NodeEditor::QueryDeletedLink(&deletedLinkId))
		{
			if (ax::NodeEditor::AcceptDeletedItem())
			{
				m_ActiveScript.DeleteLink(deletedLinkId);
			}
		}

		ax::NodeEditor::NodeId deletedNodeId = 0;
		while (ax::NodeEditor::QueryDeletedNode(&deletedNodeId))
		{
			if (ax::NodeEditor::AcceptDeletedItem() && CanNodeBeModified(deletedNodeId))
			{
				m_ActiveScript.DeleteNode(deletedNodeId);
			}
		}
	}
	ax::NodeEditor::EndDelete(); // Wrap up deletion action
}

void NodeEditor::DrawNode(Node* node)
{
	ax::NodeEditor::BeginNode(node->id);
	ImGui::PushID(node->id.AsPointer());

	// Render Header
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
		ImGui::BeginGroup();
		auto ts = ImGui::CalcTextSize(node->name.c_str());
		ImGui::Spacing();
		ImGui::TextUnformatted(node->name.c_str());
		ImGui::SameLine();
		//auto nodePadding = ax::NodeEditor::GetStyle().NodePadding;
		auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		ImGui::Dummy(ImVec2(node->size.x - ts.x - itemSpacing.x, 0));
		ImGui::Spacing();
		ImGui::EndGroup();
		ImGui::PopStyleColor();

		//std::cout << ts.x << std::endl;
	}

	ImVec2 currentHeaderMin = ImGui::GetItemRectMin();
	ImVec2 currentHeaderMax = ImGui::GetItemRectMax();

	if (ImGui::IsMouseHoveringRect(currentHeaderMin, currentHeaderMax) &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("Node options");
		LOG_INFO("Open popup");
	}


	//ImGui::BeginTable("Layout", 3, ImGuiTableFlags_Borders, ImVec2(node->size.x, 0));
	ImGui::BeginTable("Layout", 3, ImGuiTableFlags_None, ImVec2(node->size.x, 0));
	ImGui::TableSetupColumn("IN", ImGuiTableColumnFlags_WidthFixed, 30);
	ImGui::TableSetupColumn("CO", ImGuiTableColumnFlags_NoResize);
	ImGui::TableSetupColumn("OU", ImGuiTableColumnFlags_WidthFixed, 30);
	//ImGui::TableHeadersRow();

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::PushFont(UICore::GetFont(Font::BigIcons));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
	if (node->inputPin.active)
	{
		ax::NodeEditor::BeginPin(node->inputPin.id, ax::NodeEditor::PinKind::Input);
		if (m_ActiveScript.IsPinConnected(node->inputPin.id))
			ImGui::Text(ICON_MD_LABEL);
		else
			ImGui::Text(ICON_MD_LABEL_OUTLINE);
		ax::NodeEditor::EndPin();
	}
	ImGui::PopStyleColor();
	ImGui::PopFont();

	ImGui::TableNextColumn();
	node->Render();
	if (node->type == Node::Type::Scope)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID | ImGuiDragDropFlags_SourceNoPreviewTooltip))
		{
			int id = (int)node->id.AsPointer();
			ImGui::SetDragDropPayload("DND_SCOPE", &id, sizeof(int));
			ImGui::EndDragDropSource();

			m_ShowDragDropTooltip = true;
		}
		else {
			m_ShowDragDropTooltip = false;
		}
	}

	ImGui::TableNextColumn();
	ImGui::PushFont(UICore::GetFont(Font::BigIcons));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
	if (node->outputPin.active)
	{
		ax::NodeEditor::BeginPin(node->outputPin.id, ax::NodeEditor::PinKind::Output);
		if (m_ActiveScript.IsPinConnected(node->outputPin.id))
			ImGui::Text(ICON_MD_LABEL);
		else
			ImGui::Text(ICON_MD_LABEL_OUTLINE);
		ax::NodeEditor::EndPin();
	}
	ImGui::PopStyleColor();
	ImGui::PopFont();
	ImGui::EndTable();

	ImGui::PopID();
	ax::NodeEditor::EndNode();

	// Render Header Background
	{
		auto dl = ax::NodeEditor::GetNodeBackgroundDrawList(node->id);
		const auto halfBorderWidth = ax::NodeEditor::GetStyle().NodeBorderWidth * 0.5f;
		const auto alpha = static_cast<int>(255 * ImGui::GetStyle().Alpha);

		ImU32 HeaderColor = IM_COL32(66, 150, 250, 200);
		auto headerColor = IM_COL32(0, 0, 0, alpha) | (HeaderColor & IM_COL32(255, 255, 255, 0));

		dl->AddRectFilled(
			currentHeaderMin - ImVec2(8 - halfBorderWidth, 4 - halfBorderWidth),
			currentHeaderMax + ImVec2(8 - halfBorderWidth, 0),
			headerColor,
			ax::NodeEditor::GetStyle().NodeRounding,
			ImDrawFlags_RoundCornersTop);
	}
}