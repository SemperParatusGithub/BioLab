#include "NodeEditor.h"

#include "Comment.h"
#include "Source.h"
#include "Filter.h"

#include "UI/UICore.h"
#include "UI/IconsMaterialDesign.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <implot.h>

#include <stb_image.h>
#include <GLFW/glfw3.h>


NodeEditor::NodeEditor(const NodeEditorConfig& config)
{
	m_Config = config;

	ax::NodeEditor::Config internalConfig;
	internalConfig.SettingsFile = "Test.json";
	internalConfig.DragButtonIndex = 0;

	m_HeaderTextureID = UICore::LoadTexture("../../BioLab/Ressources/BlueprintBackground.png");

	m_EditorContext = ax::NodeEditor::CreateEditor(&internalConfig);

	ax::NodeEditor::SetCurrentEditor(m_EditorContext);
	ax::NodeEditor::GetStyle() = ax::NodeEditor::Style();
	ax::NodeEditor::GetStyle().FlowDuration = 1.0f;
	ax::NodeEditor::GetStyle().LinkStrength = 350.0f;
	ax::NodeEditor::SetCurrentEditor(nullptr);

	ImFontConfig fontConfig;
	fontConfig.OversampleH = 4;
	fontConfig.OversampleV = 4;
	fontConfig.PixelSnapH = false;
	m_Font = ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/Fonts/Play-Regular.ttf", 15, &fontConfig);

	m_Channel1Node = CreateNode(ICON_MD_POST_ADD"  Channel 1", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 200.0f, 80.0f });
	m_Channel2Node = CreateNode(ICON_MD_POST_ADD"  Channel 2", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 200.0f, 80.0f });
	m_Channel3Node = CreateNode(ICON_MD_POST_ADD"  Channel 3", Node::Type::Source, Vector2f{ 250.0f, 10.0f }, Vector2f{ 200.0f, 80.0f });

	m_Nodes.emplace_back(new Comment(GetNextNodeID(), "Test", Vector2f{ 0.0f, 0.0f }, Vector2f{ 220.0f, 110.0f }));
}
NodeEditor::~NodeEditor()
{
	for (auto* node : m_Nodes)
		delete node;
}

void NodeEditor::AddNewSample(const Vector4f& sample)
{
	Flow();

	// LOG_NEWLINE();
	// LOG_INFO("--------------------------");
	// LOG_INFO("Node::Editor::AddNewSample");
	// LOG_INFO("%.2f: [%.2f, %.2f, %.2f]", sample.x, sample.y, sample.z, sample.w);
	// LOG_INFO("--------------------------");

	ProcessNodeWithSample(m_Channel1Node, sample.y);
	ProcessNodeWithSample(m_Channel2Node, sample.z);
	ProcessNodeWithSample(m_Channel3Node, sample.w);

	// for (auto* node : m_Nodes)
	// {
	// 	if (node->type == Node::Type::Source)
	// 		ProcessNodeWithSample(node, sample.y);
	// }
}

void NodeEditor::Render()
{
	bool showDragTooltip = false;

	if (!m_IsOpen)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
	if (ImGui::Begin(ICON_MD_INSERT_CHART" Node Editor", &m_IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		ax::NodeEditor::SetCurrentEditor(m_EditorContext);
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
		ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
		ax::NodeEditor::Begin("Node Editor", ImGui::GetContentRegionAvail());
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

			for (auto* node : m_Nodes)
			{
				// comments are handled specially
				if (node->type == Node::Type::Comment)
				{
					node->Render();
					continue;
				}

				DrawNode(node);
			}

			ImGui::PopStyleColor();

			HandleLinks();
		}


		ax::NodeEditor::Suspend();

		ImVec2 openPopupPosition;

		if (ax::NodeEditor::ShowBackgroundContextMenu())
			ImGui::OpenPopup("Create New Node");
		else if (ax::NodeEditor::ShowNodeContextMenu(&contextNodeId))
			ImGui::OpenPopup("Node Context");
		else 
			ImGui::CloseCurrentPopup();
		

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

		if (ImGui::BeginPopup("Create New Node"))
		{
			if (ImGui::MenuItem("Comment"))
				CreateNode("Comment", Node::Type::Comment, { 100.0f, 100.0f }, { 200.0f, 100.0f });
			if (ImGui::MenuItem("Source"))
				CreateNode("Source", Node::Type::Source, { 100.0f, 100.0f }, { 200.0f, 100.0f });
			if (ImGui::MenuItem("Scope"))
				CreateNode("Scope", Node::Type::Scope, { 100.0f, 100.0f }, { 200.0f, 100.0f });
			if (ImGui::MenuItem("Filter"))
				CreateNode("Filter", Node::Type::Filter, { 500.0f, 500.0f }, { 200.0f, 200.0f });

			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("Node Context"))
		{
			if (ImGui::MenuItem("Rename"));
			if (ImGui::MenuItem("Delete"));

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
	if (ImGui::CollapsingHeader("Nodes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto* node : m_Nodes)
		{
			ImGui::Text(node->name.c_str());
			ImGui::Text("ID: %d", node->id);
			ImGui::Text("Position: %.2f, %.2f", node->position.x, node->position.y);
			ImGui::Text("Size: %.2f, %.2f", node->size.x, node->size.y);
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
		}
	}
	if (ImGui::CollapsingHeader("Links", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto& link : m_Links)
		{
			ImGui::Text("Link %d   from %d to %d", link.ID.AsPointer(), link.StartPinID.AsPointer(), link.EndPinID.AsPointer());
		}
	}

	if (ImGui::Button("Clear all nodes"))
		m_Nodes.clear();

	if (ImGui::Button("Navigate to content"))
	{
		ax::NodeEditor::SetCurrentEditor(m_EditorContext);
		ax::NodeEditor::NavigateToContent();
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}

	ImGui::End();
}

Node* NodeEditor::CreateNode(const std::string& name, Node::Type type, const Vector2f& position, const Vector2f& size)
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
	//case Node::Type::Filter:
	//{
	//	node = m_Nodes.emplace_back(new Filter(GetNextNodeID(), name, position, size));
	//	node->inputPin = Pin{ "Input1", Pin::Type::Input, GetNextPinID(), true, node };
	//	node->outputPin = Pin{ "Output1", Pin::Type::Output, GetNextPinID(), true, node };
	//	break;
	//}
	}

	return node;
}

ax::NodeEditor::NodeId NodeEditor::GetNextNodeID()
{
	m_CurrentNodeID++;
	return ax::NodeEditor::NodeId(m_CurrentNodeID);
}
ax::NodeEditor::LinkId NodeEditor::GetNextLinkID()
{
	m_CurrentLinkID++;
	return ax::NodeEditor::LinkId(m_CurrentLinkID);
}
ax::NodeEditor::PinId NodeEditor::GetNextPinID()
{
	m_CurrentPinID++;
	return ax::NodeEditor::PinId(m_CurrentPinID);
}

void NodeEditor::HandleLinks()
{
	// taken from examples

	// Submit Links
	for (auto& link : m_Links)
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

			if (CanCreateLink(inputPinId, outputPinId)) // both are valid, let's accept link
			{
				// ax::NodeEditor::AcceptNewItem() return true when user release mouse button.
				if (ax::NodeEditor::AcceptNewItem())
				{
					// Since we accepted new link, lets add one to our list of links.
					m_Links.push_back({ GetNextLinkID(), inputPinId, outputPinId });

					auto& pin1 = FindPin(inputPinId);
					auto& pin2 = FindPin(outputPinId);

					if (pin1.type == Pin::Type::Output)
						pin1.node->nextLinkedNode = pin2.node;

					if (pin2.type == Pin::Type::Output)
						pin2.node->nextLinkedNode = pin1.node;

					// Draw new link.
					ax::NodeEditor::Link(m_Links.back().ID, m_Links.back().StartPinID, m_Links.back().EndPinID);
				}

				// You may choose to reject connection between these nodes
				// by calling ax::NodeEditor::RejectNewItem(). This will allow editor to give
				// visual feedback by changing link thickness and color.
			}
		}
	}
	ax::NodeEditor::EndCreate(); // Wraps up object creation action handling.


	// Handle deletion action
	if (ax::NodeEditor::BeginDelete())
	{
		// There may be many links marked for deletion, let's loop over them.
		ax::NodeEditor::LinkId deletedLinkId;
		while (ax::NodeEditor::QueryDeletedLink(&deletedLinkId))
		{
			// If you agree that link can be deleted, accept deletion.
			if (ax::NodeEditor::AcceptDeletedItem())
			{
				m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
					[=](Link& element) -> bool
					{
						if (element.ID == deletedLinkId)
							return true;
						else return false;
					}),
					m_Links.end());
			}

			// You may reject link deletion by calling:
			// ed::RejectDeletedItem();
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


	ImGui::BeginTable("Layout", 3, ImGuiTableFlags_Borders, ImVec2(node->size.x, 0));
	ImGui::TableSetupColumn("IN", ImGuiTableColumnFlags_WidthFixed, 30);
	ImGui::TableSetupColumn("CO", ImGuiTableColumnFlags_NoResize);
	ImGui::TableSetupColumn("OU", ImGuiTableColumnFlags_WidthFixed, 30);
	//ImGui::TableHeadersRow();

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
	if (node->inputPin.active)
	{
		ax::NodeEditor::BeginPin(node->inputPin.id, ax::NodeEditor::PinKind::Input);
		if (IsPinConnected(node->inputPin.id))
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
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
	if (node->outputPin.active)
	{
		ax::NodeEditor::BeginPin(node->outputPin.id, ax::NodeEditor::PinKind::Output);
		if (IsPinConnected(node->outputPin.id))
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



#if defined(BIOLAB_GLFW_OPENGL3)
		dl->AddImageRounded(
			(ImTextureID)m_HeaderTextureID,
			currentHeaderMin - ImVec2(8 - halfBorderWidth, 4 - halfBorderWidth),
			currentHeaderMax + ImVec2(8 - halfBorderWidth, 0),
			ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f),
			headerColor, ax::NodeEditor::GetStyle().NodeRounding,
			ImDrawFlags_RoundCornersTop);
#else
		dl->AddRectFilled(
			currentHeaderMin - ImVec2(8 - halfBorderWidth, 4 - halfBorderWidth),
			currentHeaderMax + ImVec2(8 - halfBorderWidth, 0),
			headerColor,
			ax::NodeEditor::GetStyle().NodeRounding,
			ImDrawFlags_RoundCornersTop);
#endif
	}
}