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

	m_Window = std::make_unique<Window>("BioLab", 1280, 720);
	m_Window->SetMinimumSize(Vector2f{ 1280.0f, 720.0f });
	//m_Window->Maximize();

	//const auto* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	//m_Window->SetPosition(Vector2f{ vidmode->width / 2.0f - 640.0f, vidmode->height / 2.0f - 360.0f });

	static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
	ImFontConfig icons_config; 
	icons_config.MergeMode = true; 
	icons_config.PixelSnapH = true;
	ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/Fonts/OpenSans/OpenSans-Regular.ttf", 20.0f);
	ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/MaterialIcons-Regular.ttf", 17.0f, &icons_config, icons_ranges);

	m_BigIcons = ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/MaterialIcons-Regular.ttf", 42, 0, icons_ranges);

	m_NodeEditor = std::make_unique<NodeEditor>();

	m_ReaderThread = std::thread(&Application::ReadSerialPort, this);
	glfwSwapInterval(0);
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

	m_LiveValuesX.push_back(-0.1f);
	m_LiveValuesCH1.push_back(0.0f);
	m_LiveValuesCH2.push_back(0.0f);
	m_LiveValuesCH3.push_back(0.0f);

	while (m_Window->isOpen())
	{
		m_Window->BeginFrame();

		m_Window->Update();
		m_Window->Clear(Vector4f { 1.0f, 0.0f, 1.0f, 1.0f });

		// Retrieve data from ReaderThread
		{
			m_InputQueueMutex.lock();
			while (!m_InputQueue.empty())
			{
				auto& sample = m_InputQueue.front();
				m_LiveValuesX.push_back(sample.x);
				m_LiveValuesCH1.push_back(sample.y);
				m_LiveValuesCH2.push_back(sample.z);
				m_LiveValuesCH3.push_back(sample.w);
			
				m_InputQueue.pop();
			}
			m_InputQueueMutex.unlock();
		}

		if (m_LiveValuesX.size() >= 1000)
		{
			m_LiveValuesX.erase(m_LiveValuesX.begin(), m_LiveValuesX.begin() + 250);
			m_LiveValuesCH1.erase(m_LiveValuesCH1.begin(), m_LiveValuesCH1.begin() + 250);
			m_LiveValuesCH2.erase(m_LiveValuesCH2.begin(), m_LiveValuesCH2.begin() + 250);
			m_LiveValuesCH3.erase(m_LiveValuesCH3.begin(), m_LiveValuesCH3.begin() + 250);
		}

		bool minimized = m_Window->GetSize().x == 0 && m_Window->GetSize().y == 0;
		if (!minimized)
		{
			BeginDockspace();

			static bool plotWindowOpen = true;
			if (plotWindowOpen)
			{
				if (ImGui::Begin(ICON_MD_INSERT_CHART" Plot Window", &plotWindowOpen, ImGuiWindowFlags_NoCollapse))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					ImPlot::SetNextAxisLimits(ImAxis_Y1, -2000, 6500, ImGuiCond_FirstUseEver);
					ImPlot::SetNextAxisLimits(ImAxis_X1, (float)m_LiveValuesX.back() - 4.5f, (float)m_LiveValuesX.back() + 0.5f, ImGuiCond_Always);
					ImPlot::BeginPlot("My Arduino Data", 0, 0, ImGui::GetContentRegionAvail());
					ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
					ImPlot::PlotLine("CH1", m_LiveValuesX.data(), m_LiveValuesCH1.data(), m_LiveValuesX.size());
					ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
					ImPlot::PlotLine("CH2", m_LiveValuesX.data(), m_LiveValuesCH2.data(), m_LiveValuesX.size());
					ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
					ImPlot::PlotLine("CH3", m_LiveValuesX.data(), m_LiveValuesCH3.data(), m_LiveValuesX.size());
					ImPlot::EndPlot();
					ImGui::PopStyleVar();
				}
				ImGui::End();
			}

			static bool nodeEditorOpen = true;
			if (nodeEditorOpen)
			{
				if (ImGui::Begin(ICON_MD_INSERT_CHART" Bar Plot", &nodeEditorOpen, ImGuiWindowFlags_NoCollapse))
				{
					m_NodeEditor->Render(ImGui::GetContentRegionAvail());
				}
				ImGui::End();
			}

			m_NodeEditor->ShowDebugWindow();
			ImGui::ShowDemoWindow();

			EndDockspace();
		}

		m_Window->EndFrame();
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
	ImGui::Text("Mouse Pos: %.2f, %.2f", ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	ImGui::Separator();

	static bool lightColors = false;
	if (ImGui::Checkbox("Light Colors", &lightColors))
		lightColors ? UICore::SetLightColorTheme() : UICore::SetDarkColorTheme();
	ImGui::Separator();

	ImGui::Text("SideBar Pos: %.2f, %.2f", sideBarPos.x, sideBarPos.y);
	ImGui::Text("SideBar Size: %.2f, %.2f", sideBarSize.x, sideBarSize.y);
	ImGui::Separator();
	ImGui::Text("MenuBar Pos: %.2f, %.2f", menuBarPos.x, menuBarPos.y);
	ImGui::Text("MenuBar Size: %.2f, %.2f", menuBarSize.x, menuBarSize.y);
	ImGui::Separator();
	ImGui::Text("Dockspace Pos: %.2f, %.2f", dockspacePos.x, dockspacePos.y);
	ImGui::Text("Dockspace Size: %.2f, %.2f", dockspaceSize.x, dockspaceSize.y);

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