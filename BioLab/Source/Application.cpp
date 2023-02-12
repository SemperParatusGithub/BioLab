#include "Application.h"

#include "Core.h"
#include "UI/UICore.h"

#include <imgui.h>
#include <implot.h>

#include <GLFW/glfw3.h>

#include "UI/IconsMaterialDesign.h"

typedef struct
{
	uint32_t sampleIndex;			// Number of sample
	uint16_t ch0, ch1, ch2;			// Sample values
} DataPackage;


Application::Application()
{
	LOG_INFO("creating Application");

	UICore::Initialize();
	UICore::SetupStyle();
	UICore::SetLightColorTheme();

	static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
	ImFontConfig icons_config; 
	icons_config.MergeMode = true; 
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 4;
	icons_config.OversampleV = 4;

	ImFontConfig icons_config1; 
	icons_config1.OversampleH = 4;
	icons_config1.OversampleV = 4;
	ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/Fonts/OpenSans/OpenSans-Regular.ttf", 20.0f, &icons_config1);
	ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/MaterialIcons-Regular.ttf", 20.0f, &icons_config, icons_ranges);

	m_BigIcons = ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/MaterialIcons-Regular.ttf", 30, 0, icons_ranges);

	m_NodeEditor = std::make_unique<NodeEditor>();
	m_NodeEditor->SetupStyle();

	m_ReaderThread = std::thread(&Application::ReadSerialPort, this);
}

Application::~Application()
{
	LOG_INFO("shutting down Application");

	m_SerialThreadRunning = false;
	m_ReaderThread.join();
}

void Application::Run()
{
	LOG_INFO("running Application");

	while (UICore::WindowOpen())
	{
		UICore::BeginFrame();

		// Retrieve data from ReaderThread
		{
			m_InputQueueMutex.lock();
			while (!m_InputQueue.empty())
			{
				auto& sample = m_InputQueue.front();
				m_LiveValuesX.PushBack(sample.x);
				m_LiveValuesCH1.PushBack(sample.y);
				m_LiveValuesCH2.PushBack(sample.z);
				m_LiveValuesCH3.PushBack(sample.w);

				m_NodeEditor->AddNewSample(sample);
			
				m_InputQueue.pop();
			}
			m_InputQueueMutex.unlock();
		}

		bool minimized = false; // m_Window->GetSize().x == 0 && m_Window->GetSize().y == 0;
		if (!minimized)
		{
			BeginDockspace();

			static bool plotWindowOpen = true;
			if (plotWindowOpen)
			{
				if (ImGui::Begin(ICON_MD_INSERT_CHART" Plot Window", &plotWindowOpen, ImGuiWindowFlags_NoCollapse))
				{
					if (m_LiveValuesX.Size() != 0)
					{
						ImPlot::SetNextAxisLimits(ImAxis_Y1, -2000, 6500, ImGuiCond_FirstUseEver);
						float xMin = (float)m_LiveValuesX.Back() - 10.5f;
						float xMax = (float)m_LiveValuesX.Back() + 0.5f;
						ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);

						ImPlot::BeginSubplots("", 3, 1, ImGui::GetContentRegionAvail(), ImPlotSubplotFlags_LinkAllX);

						ImPlot::BeginPlot("CH1", 0, 0, ImGui::GetContentRegionAvail());
						ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
						ImPlot::PlotLine("data", m_LiveValuesX.Data(), m_LiveValuesCH1.Data(), m_LiveValuesX.Size());
						ImPlot::EndPlot();

						ImPlot::BeginPlot("CH2", 0, 0, ImGui::GetContentRegionAvail());
						ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
						ImPlot::PlotLine("data", m_LiveValuesX.Data(), m_LiveValuesCH2.Data(), m_LiveValuesX.Size());
						ImPlot::EndPlot();

						ImPlot::BeginPlot("CH3", 0, 0, ImGui::GetContentRegionAvail());
						ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
						ImPlot::PlotLine("data", m_LiveValuesX.Data(), m_LiveValuesCH3.Data(), m_LiveValuesX.Size());
						ImPlot::EndPlot();

						ImPlot::EndSubplots();
					}
				}
				ImGui::End();
			}

			static bool scopeWindowOpen = true;
			if (scopeWindowOpen)
			{
				if (ImGui::Begin(ICON_MD_INSERT_CHART" Scope Window", &scopeWindowOpen, ImGuiWindowFlags_NoCollapse))
				{
					if (m_LiveValuesX.Size() != 0)
					{
						ImPlot::SetNextAxisLimits(ImAxis_Y1, -2000, 6500, ImGuiCond_FirstUseEver);
						float xMin = (float)m_LiveValuesX.Back() - 10.5f;
						float xMax = (float)m_LiveValuesX.Back() + 0.5f;
						ImPlot::SetNextAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
					}

					ImPlot::BeginPlot("CH1", 0, 0, ImGui::GetContentRegionAvail());
					if (ImPlot::BeginDragDropTargetPlot()) 
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SCOPE");
						if (payload)
						{
							int id = *(int*)payload->Data;
							LOG_INFO("NodeID: %d", id);

							if (std::find(m_ActiveScopes.begin(), m_ActiveScopes.end(), id) == m_ActiveScopes.end())
								m_ActiveScopes.push_back(id);
						}
						ImPlot::EndDragDropTarget();
					}

					for (auto& id : m_ActiveScopes)
					{
						Node* node = m_NodeEditor->FindNodeByID(ax::NodeEditor::NodeId(id));
						if (node != nullptr)
						{
							Scope* scopeNode = reinterpret_cast<Scope*>(node);
							std::string scopeName = scopeNode->name;
							auto& samples = scopeNode->Samples;

							if (m_LiveValuesX.Size() != 0)
							{
								ImPlot::PlotLine(scopeName.c_str(), m_LiveValuesX.Data(), scopeNode->Samples.Data(), scopeNode->Samples.Size());
							}
						}
					}


					ImPlot::EndPlot();
				}
				ImGui::End();
			}

			m_NodeEditor->Render();			
			// m_NodeEditor->ShowDebugWindow();
			//
			ImGui::ShowDemoWindow();
			// ImPlot::ShowDemoWindow();


			EndDockspace();
		}

		UICore::EndFrame();
		UICore::PollEvents();
	}
}

void Application::ReadSerialPort()
{
	std::cout << "Starting Serial Reader thread" << std::endl;

	m_SerialPort.Open(ARDUINO_PORT, 1000000);
	if (!m_SerialPort.IsConnected())
		return;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	m_SerialPort.ClearQueue();

	std::cout << "Ready to receive data" << std::endl;

	auto start = std::chrono::high_resolution_clock::now();
	std::size_t samplesReceived = 0;

	int idx = 0;

	while (m_SerialThreadRunning)
	{
		char key;
		if (m_SerialPort.Read(&key, 1) && key == 120)
		{
			DataPackage package;
			if (m_SerialPort.Read((void*)&package, 10))
			{
				samplesReceived++;

				if (idx <= 20)
				{
					idx++;
					continue;
				}

				idx = 0;
				Vector4f sample = { 
					package.sampleIndex / 2000.0f, 
					package.ch0, 
					package.ch1, 
					package.ch2 
				};

				const std::lock_guard<std::mutex> lock(m_InputQueueMutex);
				m_InputQueue.push(sample);
				// Lock guard is released when going out of scope
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	float duration_s = duration / 1e6f;

	std::cout << "Stopping Serial Reader thread" << std::endl;
	std::cout << "Duration: " << duration_s << std::endl;
	std::cout << "Samples: " << samplesReceived << std::endl;
	std::cout << "samples/s: " << samplesReceived / duration_s << std::endl;
}

void Application::BeginDockspace()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImVec2 sideBarPos = viewport->Pos;
	ImVec2 sideBarSize = ImVec2{ 1.0f / 5.0f * viewport->Size.x, viewport->Size.y };

	ImVec2 menuBarPos = ImVec2{ viewport->Pos.x + sideBarSize.x, viewport->Pos.y };
	ImVec2 menuBarSize = ImVec2{ 4.0f / 5.0f * viewport->Size.x, 55.0f };

	ImVec2 dockspacePos = ImVec2{ viewport->Pos.x + sideBarSize.x, viewport->Pos.y + menuBarSize.y };
	ImVec2 dockspaceSize = ImVec2{ 4.0f / 5.0f * viewport->Size.x, viewport->Size.y - menuBarSize.y };


	// Left window
	ImGui::SetNextWindowPos(sideBarPos);
	ImGui::SetNextWindowSize(sideBarSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 5.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Begin("LeftBar", 0, windowFlags);
	ImGui::BeginChild("Child", ImVec2(-1, -1), true);

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
	ImGui::Text("Mouse Pos: %.2f, %.2f", ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	ImGui::Separator();

	//static bool flow = false;
	//ImGui::Checkbox("Flow", &flow);
	//if (flow)
	//	m_NodeEditor->Flow();

	static bool lightColors = false;
	// if (ImGui::Checkbox("Light Colors", &lightColors))
	// 	lightColors ? UICore::SetLightColorTheme() : UICore::SetDarkColorTheme();
	ImGui::Separator();

	if (ImGui::Button(ICON_MD_START, ImVec2(25.0f, 25.0f)))
	{
		m_SerialPort.ClearQueue();
		char buf = 1;
		m_SerialPort.Write(&buf, 1);
	}
	if (ImGui::Button(ICON_MD_STOP, ImVec2(25.0f, 25.0f)))
	{
		m_SerialPort.ClearQueue();
		char buf = 0;
		m_SerialPort.Write(&buf, 1);
	}

	ImGui::Text("Hello");

	ImGui::Separator();
	ImGui::Indent();
	auto& scopes = m_NodeEditor->GetScopes();
	for (auto& scope : scopes)
	{
		ImGui::Selectable(scope->name.c_str());
	}
	ImGui::Unindent();

	ImGui::PopStyleColor();

	ImGui::EndChild();
	ImGui::End();
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor();

	// MenuBar
	ImGui::SetNextWindowPos(menuBarPos);
	ImGui::SetNextWindowSize(menuBarSize);	
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 5.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Begin("MenuBar", 0, windowFlags);
	ImVec2 childSize = { menuBarSize.x - 10.0f, menuBarSize.y - 2.0f * 5.0f };
	ImGui::BeginChild("Child", childSize, true);
	ImGui::EndChild();
	ImGui::End();
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor();

	ImGui::SetNextWindowPos(dockspacePos);
	ImGui::SetNextWindowSize(dockspaceSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Begin("DockSpace", 0, windowFlags);

	ImVec2 childSize2 = ImVec2{ dockspaceSize.x - 10.0f, dockspaceSize.y - 10.0f };
	ImVec2 dockspaceSize2 = ImVec2{ dockspaceSize.x - 15.0f, dockspaceSize.y - 15.0f };
	ImGui::BeginChild("Child", childSize2, true);
	ImGuiID dockSpaceID = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockSpaceID, childSize2, ImGuiDockNodeFlags_None);
	ImGui::EndChild();

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor();
}

void Application::EndDockspace()
{
	ImGui::End();	
}