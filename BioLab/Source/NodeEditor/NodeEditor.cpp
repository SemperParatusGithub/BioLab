#include "NodeEditor.h"

#include "Comment.h"
#include "Source.h"
#include "Scope.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>


static Pin* newLinkPin = nullptr;


NodeEditor::NodeEditor(const NodeEditorConfig& config)
{
	m_Config = config;

	ax::NodeEditor::Config internalConfig;
	internalConfig.SettingsFile = "Test.json";

	m_EditorContext = ax::NodeEditor::CreateEditor(&internalConfig);

	ax::NodeEditor::SetCurrentEditor(m_EditorContext);
	ax::NodeEditor::GetStyle() = ax::NodeEditor::Style();
	ax::NodeEditor::GetStyle().FlowDuration = 0.1f;
	ax::NodeEditor::SetCurrentEditor(nullptr);

	ImFontConfig fontConfig;
	fontConfig.OversampleH = 4;
	fontConfig.OversampleV = 4;
	fontConfig.PixelSnapH = false;
	m_Font = ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/Fonts/Play-Regular.ttf", 15, &fontConfig);

	m_Nodes.push_back(new Comment(GetNextNodeID(), "Test Comment", Vector2f{ 0.0f, 0.0f }, Vector2f{ 500.0f, 100.0f }));

	m_Nodes.push_back(new Source(GetNextNodeID(), "Test Source 1", Vector2f{ 100.0f, 10.0f }, Vector2f{ 100.0f, 80.0f }));
	m_Nodes.back()->GetOutputPins().push_back(Pin{ "Input Pin", ax::NodeEditor::PinId(GetNextPinID()), PinKind::Output, m_Nodes.back() });

	m_Nodes.push_back(new Source(GetNextNodeID(), "Test Source 2", Vector2f{ 100.0f, 10.0f }, Vector2f{ 100.0f, 80.0f }));
	m_Nodes.back()->GetOutputPins().push_back(Pin{ "Input Pin", ax::NodeEditor::PinId(GetNextPinID()), PinKind::Output, m_Nodes.back() });

	m_Nodes.push_back(new Scope(GetNextNodeID(), "Test Scope", Vector2f{ 300.0f, 10.0f }, Vector2f{ 100.0f, 80.0f }));
	m_Nodes.back()->GetInputPins().push_back(Pin{ "Input Pin", ax::NodeEditor::PinId(GetNextPinID()), PinKind::Input, m_Nodes.back() });
}
NodeEditor::~NodeEditor()
{
	for (auto* node : m_Nodes)
		delete node;
}

void NodeEditor::Render(const ImVec2& size)
{
	ax::NodeEditor::SetCurrentEditor(m_EditorContext);
	ImGui::PushFont(m_Font);

	ax::NodeEditor::Begin("Node Editor", ImGui::GetContentRegionAvail());

	for (auto& node : m_Nodes)
	{
		// comments are handled specially
		if (node->GetNodeType() == NodeType::Comment)
		{
			node->Render();
			continue;
		}

		ax::NodeEditor::BeginNode(node->GetNodeID());

		// Name/title
		ImGui::Text("%s (ID: %d)", node->GetName().c_str(), node->GetNodeID().Get());

		// Inputs and Outputs
		ImGui::Text("[Inputs]");
		for (auto& input : node->GetInputPins())
		{
			auto alpha = ImGui::GetStyle().Alpha;
			if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
				alpha = alpha * (48.0f / 255.0f);

			ax::NodeEditor::BeginPin(input.ID, ax::NodeEditor::PinKind::Input);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ImGui::Text("IN"); ImGui::SameLine();
			ax::NodeEditor::EndPin();
			ImGui::SameLine();
			ImGui::TextUnformatted(input.Name.c_str());
			ImGui::PopStyleVar();
		}

		ImGui::Text("[Outputs]");
		for (auto& output : node->GetOutputPins())
		{
			auto alpha = ImGui::GetStyle().Alpha;
			if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
				alpha = alpha * (48.0f / 255.0f);

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ImGui::TextUnformatted(output.Name.c_str());
			ax::NodeEditor::BeginPin(output.ID, ax::NodeEditor::PinKind::Output);
			ImGui::Text("OUT");
			ax::NodeEditor::EndPin();
			ImGui::PopStyleVar();
		}

		node->Render();

		ax::NodeEditor::EndNode();
	}

	HandleLinks();

	ax::NodeEditor::End();

	ImGui::PopFont();
	ax::NodeEditor::SetCurrentEditor(nullptr);
}

void NodeEditor::ShowDebugWindow()
{
	ImGui::Begin("Node Editor Debug");
	if (ImGui::CollapsingHeader("Nodes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto* node : m_Nodes)
		{
			ImGui::Text(node->GetName().c_str());
			ImGui::Text("ID: %d", node->GetNodeID());
			ImGui::Text("Position: %.2f, %.2f", node->GetPosition().x, node->GetPosition().y);
			ImGui::Text("Size: %.2f, %.2f", node->GetSize().x, node->GetSize().y);
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
	ImGui::End();
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

bool NodeEditor::CanCreateLink(Pin* a, Pin* b)
{
	if (!a || !b || a == b || a->Kind == b->Kind || a->Node == b->Node)
		return false;

	return true;
}

void NodeEditor::HandleLinks()
{
	// Submit Links
	for (auto& linkInfo : m_Links)
		ax::NodeEditor::Link(linkInfo.ID, linkInfo.StartPinID, linkInfo.EndPinID);


	// Handle creation action, returns true if editor want to create new object (node or link)
	if (ax::NodeEditor::BeginCreate())
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

			if (inputPinId && outputPinId) // both are valid, let's accept link
			{
				// ax::NodeEditor::AcceptNewItem() return true when user release mouse button.
				if (ax::NodeEditor::AcceptNewItem())
				{
					// Since we accepted new link, lets add one to our list of links.
					m_Links.push_back({ GetNextLinkID(), inputPinId, outputPinId });

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