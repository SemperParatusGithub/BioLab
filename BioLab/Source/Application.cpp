#include "Application.h"

#include "UI/UICore.h"
#include "UI/IconsMaterialDesign.h"

#include "NodeEditor/Nodes/LiveNodes.h"

#include "Util/FileUtils.h"


typedef struct
{
	uint32_t sampleIndex;			// Number of sample
	uint16_t ch0, ch1, ch2;			// Sample values
} DataPackage;



Application* Application::s_Instance = nullptr;

Application::Application()
{
	LOG_INFO("creating Application");

	s_Instance = this;

	UICore::Initialize();
	UICore::LoadFonts();
	UICore::SetupStyle();
	//UICore::SetLightColorTheme();
	UICore::SetDarkColorTheme();

	m_NodeEditor = std::make_unique<NodeEditor>();
	m_NodeEditor->SetupStyle();
	m_NodeEditor->SetDarkColorTheme();

	m_ReaderThread = std::thread(&Application::SimulateReadSerialPort, this);
	//m_ReaderThread = std::thread(&Application::ReadSerialPort, this);

	m_PlotWindow.SetName("Plot Window 1");
	m_PlotWindow2.SetName("Plot Window 2");
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

		BeginDockspace();

		m_NodeEditor->Render();		
		m_DefaultPlotWindow.Render();
		m_PlotWindow.Render();
		m_PlotWindow2.Render();

		// ImGui::ShowDemoWindow();
		ImPlot::ShowDemoWindow();

		EndDockspace();

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

				m_OutputSignalCH1.xValues.push_back(package.sampleIndex / 2000.0f);
				m_OutputSignalCH2.xValues.push_back(package.sampleIndex / 2000.0f);
				m_OutputSignalCH3.xValues.push_back(package.sampleIndex / 2000.0f);

				m_OutputSignalCH1.yValues.push_back(package.ch0);
				m_OutputSignalCH2.yValues.push_back(package.ch1);
				m_OutputSignalCH3.yValues.push_back(package.ch2);

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
void Application::SimulateReadSerialPort()
{
	std::cout << "Starting Simulate Serial Reader thread" << std::endl;

	std::this_thread::sleep_for(std::chrono::seconds(2));
	m_SerialPort.ClearQueue();

	std::cout << "Ready to receive data" << std::endl;

	auto start = std::chrono::high_resolution_clock::now();
	std::size_t samplesReceived = 0;

	int sampleIndex = 0;

	while (m_SerialThreadRunning)
	{
		if (!m_Reading)
			continue;

		::LARGE_INTEGER ft;
		int us = 10000;
		ft.QuadPart = -static_cast<int_fast64_t>(us * 10);  // '-' using relative time

		::HANDLE timer = ::CreateWaitableTimer(NULL, TRUE, NULL);
		::SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
		::WaitForSingleObject(timer, INFINITE);
		::CloseHandle(timer);

		Vector4f sample = {
			sampleIndex / 2000.0f,
			2.0f * std::sin(5.0f * sampleIndex / 2000.0f),
			1.5f * std::cos(7.5f * sampleIndex / 2000.0f),
			std::sin(15.0f * sampleIndex / 2000.0f),
		};

		sampleIndex += 20;

		{
			const std::lock_guard<std::mutex> lock(m_InputQueueMutex);
			m_InputQueue.push(sample);
			// Lock guard is released when going out of scope
		}

		m_OutputSignalCH1.xValues.push_back(sample.x);
		m_OutputSignalCH2.xValues.push_back(sample.x);
		m_OutputSignalCH3.xValues.push_back(sample.x);

		m_OutputSignalCH1.yValues.push_back(sample.y);
		m_OutputSignalCH2.yValues.push_back(sample.z);
		m_OutputSignalCH3.yValues.push_back(sample.w);
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

	ImGuiViewport *viewport = ImGui::GetMainViewport();

	ImVec2 sideBarPos = viewport->Pos;
	ImVec2 sideBarSize = ImVec2 { 1.0f / 5.0f * viewport->Size.x, viewport->Size.y };

	ImVec2 menuBarPos = ImVec2 { viewport->Pos.x + sideBarSize.x, viewport->Pos.y };
	ImVec2 menuBarSize = ImVec2 { 4.0f / 5.0f * viewport->Size.x, 55.0f };

	ImVec2 dockspacePos = ImVec2 { viewport->Pos.x + sideBarSize.x, viewport->Pos.y + menuBarSize.y };
	ImVec2 dockspaceSize = ImVec2 { 4.0f / 5.0f * viewport->Size.x, viewport->Size.y - menuBarSize.y };

	// Left window
	ImGui::SetNextWindowPos(sideBarPos);
	ImGui::SetNextWindowSize(sideBarSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 5.0f));
	ImGui::Begin("LeftBar", 0, windowFlags);
	ImGui::BeginChild("Child", ImVec2(-1, -1), true);

	ImGui::Spacing();
	ImGui::PushFont(UICore::GetFont(Font::BigIcons));
	ImGui::TextUnformatted(ICON_MD_LOCAL_HOSPITAL);
	ImGui::SameLine();
	ImGui::PopFont();
	ImGui::PushFont(UICore::GetFont(Font::OpenSansTitle));
	ImGui::TextUnformatted("BioLab Software");
	ImGui::PopFont();
	ImGui::Spacing();

	float width = ImGui::GetContentRegionAvail().x;
	static bool statisticsOpen = false;
	ImGui::BeginChild("Statistics", ImVec2(width, statisticsOpen ? 125 : 45), true);
	ImGui::PushFont(UICore::GetFont(Font::OpenSansHeader));
	if (ImGui::CollapsingHeader("Statistics"))
	{
		ImGui::PushFont(UICore::GetFont(Font::OpenSans));
		statisticsOpen = true;
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
		ImGui::Text("Mouse Position: %.2f, %.2f", ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		ImGui::Text("LiveBuffer Size: %d", m_LiveValuesX.Size());
		ImGui::PopFont();
	}
	else {
		statisticsOpen = false;
	}
	ImGui::PopFont();
	ImGui::EndChild();

	static bool liveDataOpen = false;
	ImGui::BeginChild("LiveDataChild", ImVec2(width, liveDataOpen ? 400 : 45), true);
	{
		ImGui::PushFont(UICore::GetFont(Font::OpenSansHeader));

		if (ImGui::CollapsingHeader("Live Analysis"))
		{
			ImGui::PushFont(UICore::GetFont(Font::OpenSans));

			liveDataOpen = true;
			static int amplification[3] = { 100, 100, 100 };

			ImGui::Separator();
			ImGui::PushFont(UICore::GetFont(Font::OpenSansHeading));
			ImGui::TextUnformatted("Amplification");
			ImGui::PopFont();
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.0f);
			if (ImGui::Button(ICON_MD_SEND, ImVec2(30, 23)))
			{
				LOG_INFO("Sending Values: [%d, %d, %d]", amplification[0], amplification[1], amplification[2]);

				unsigned char MAG_BYTE = 121;
				unsigned char writeData[4] = { MAG_BYTE, amplification[0], amplification[1], amplification[2] };

				// m_SerialPort.Write uses WriteFile under the hood which is thread safe so there is no problem!!!
				m_SerialPort.Write((void*)writeData, 4);
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Apply values to potentiometer");

			ImGui::Separator();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
			ImGui::SliderInt("CH1", &amplification[0], 0, 255);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
			ImGui::SliderInt("CH2", &amplification[1], 0, 255);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
			ImGui::SliderInt("CH3", &amplification[2], 0, 255);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::PushFont(UICore::GetFont(Font::OpenSansHeading));
			ImGui::TextUnformatted("Script");
			ImGui::PopFont();
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 93.0f);

			if (ImGui::Button(ICON_MD_SAVE, ImVec2(30, 23)))
				m_NodeEditor->SaveLiveScript(UICore::SaveFileDialog("Live Script(*.livescript)\0*.livescript\0"));
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Save Script");
			ImGui::SameLine();

			if (ImGui::Button(ICON_MD_FILE_OPEN, ImVec2(30, 23)))
				m_NodeEditor->LoadLiveScript(UICore::OpenFileDialog("Live Script(*.livescript)\0*.livescript\0"));
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Load Script");
			ImGui::SameLine();

			if (ImGui::Button(ICON_MD_ADD, ImVec2(30, 23)))
				m_NodeEditor->CreateLiveScript();
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Create new Script");
			ImGui::Separator();

			ImGui::TextUnformatted("No active Script");

			auto dif = std::chrono::high_resolution_clock::now() - m_RecordingStart;
			float elapsed = m_Recording ? dif.count() / 1e9 : 0.0f;

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::PushFont(UICore::GetFont(Font::OpenSansHeading));
			ImGui::TextUnformatted("Recording");
			ImGui::PopFont();
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 55.0f);
			if (ImGui::Button(ICON_MD_START, ImVec2(30, 23)))
			{
				m_Recording = true;
				m_RecordingStart = std::chrono::high_resolution_clock::now();

				std::time_t now = std::time(nullptr);  // get current time
				char buffer[80];
				std::strftime(buffer, 80, "%Y-%m-%d_%H-%M-%S", std::localtime(&now));
				std::cout << "Current date and time: " << buffer << std::endl;
				m_OutputSignalCH1.label = "CH1_" + std::string(buffer);
				m_OutputSignalCH2.label = "CH2_" + std::string(buffer);
				m_OutputSignalCH3.label = "CH3_" + std::string(buffer);
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Start Recording");
			ImGui::SameLine();
			if (ImGui::Button(ICON_MD_STOP, ImVec2(30, 23)) || elapsed > m_RecordingDuration && m_Recording)
			{
				m_Recording = false;
				m_OutputSignalCH1.id = FileUtils::GetNextSignalID();
				m_OutputSignalCH2.id = FileUtils::GetNextSignalID();
				m_OutputSignalCH3.id = FileUtils::GetNextSignalID();

				m_OutputSignalCH1.color = FileUtils::GetNextColor();
				m_OutputSignalCH2.color = FileUtils::GetNextColor();
				m_OutputSignalCH3.color = FileUtils::GetNextColor();

				m_LoadedSignals.push_back(m_OutputSignalCH1);
				m_LoadedSignals.push_back(m_OutputSignalCH2);
				m_LoadedSignals.push_back(m_OutputSignalCH3);

				FileUtils::SaveCSV(m_OutputSignalCH1, m_OutputSignalCH1.label + ".csv");
				FileUtils::SaveCSV(m_OutputSignalCH2, m_OutputSignalCH2.label + ".csv");
				FileUtils::SaveCSV(m_OutputSignalCH3, m_OutputSignalCH3.label + ".csv");

				m_OutputSignalCH1 = Signal();
				m_OutputSignalCH2 = Signal();
				m_OutputSignalCH3 = Signal();
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Stop Recording");
			ImGui::Separator();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75.0f);
			ImGui::SliderFloat("Duration", &m_RecordingDuration, 5.0f, 20.0f);
			ImGui::ProgressBar(elapsed / m_RecordingDuration, ImVec2(ImGui::GetContentRegionAvail().x, 40.0f));
			ImGui::PopFont(); // OpenSans
		}
		else {
			liveDataOpen = false;
		}
	}
	ImGui::PopFont();
	ImGui::EndChild();

	static bool processDataOpen = false;
	ImGui::BeginChild("ProcessDataChild", ImVec2(width, processDataOpen ? 400 : 45), true);
	ImGui::PushFont(UICore::GetFont(Font::OpenSansHeader));
	if (ImGui::CollapsingHeader("Data Analysis"))
	{
		processDataOpen = true;
		ImGui::PushFont(UICore::GetFont(Font::OpenSans));

		ImGui::Separator();
		ImGui::PushFont(UICore::GetFont(Font::OpenSansHeading));
		ImGui::TextUnformatted("Loaded Signals");
		ImGui::PopFont();
		ImGui::Separator();

		if (ImGui::BeginChild("LoadedSignals + Options", ImVec2(width * 0.25f, width * 0.5f)))
		{
			if (ImGui::Button("Load", ImVec2(-1, 0)))
			{
				std::string path = UICore::OpenFileDialog("Data file (*.csv)");
				auto& signal = FileUtils::LoadCSV(path);
				m_LoadedSignals.push_back(signal);
			}
			if (ImGui::Button("Clear", ImVec2(-1, 0)))
			{
				m_LoadedSignals.clear();
			}
			ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild("Signals", ImVec2(0.0f, width * 0.5f), true);
			for (auto& signal : m_LoadedSignals)
			{
				bool sel = false;
				ImPlot::ItemIcon(signal.color);
				ImGui::SameLine();
				float availX = ImGui::GetContentRegionAvail().x;
				ImGui::Selectable(signal.label.c_str(), false, 0, ImVec2(availX - 35.0f, 0.0f));
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					int id = signal.id;
					ImGui::SetDragDropPayload("DND_SIGNAL", &id, sizeof(int));
					ImPlot::ItemIcon(signal.color); ImGui::SameLine();
					ImGui::TextUnformatted(signal.label.c_str());

					ImGui::EndDragDropSource();
				}


				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
				std::string buttonName = ICON_MD_SAVE"##" + std::to_string(signal.id);
				if (ImGui::Button(buttonName.c_str(), ImVec2(30, 20)))
				{
					std::string filepath = UICore::SaveFileDialog("Data file (*.csv)\0.csv");
					FileUtils::SaveCSV(signal, filepath);
				}

				ImGui::PopStyleColor(3);
			}
		}
		ImGui::EndChild();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::PushFont(UICore::GetFont(Font::OpenSansHeading));
		ImGui::TextUnformatted("Script");
		ImGui::PopFont();
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 93.0f);

		if (ImGui::Button(ICON_MD_SAVE, ImVec2(30, 23)))
			m_NodeEditor->SaveLiveScript(UICore::SaveFileDialog("Live Script(*.livescript)\0*.livescript\0"));
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Save Script");
		ImGui::SameLine();

		if (ImGui::Button(ICON_MD_FILE_OPEN, ImVec2(30, 23)))
			m_NodeEditor->LoadLiveScript(UICore::OpenFileDialog("Live Script(*.livescript)\0*.livescript\0"));
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Load Script");
		ImGui::SameLine();

		if (ImGui::Button(ICON_MD_ADD, ImVec2(30, 23)))
			m_NodeEditor->CreatePostProcessScript();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Create Script");
		ImGui::Separator();

		ImGui::TextUnformatted("No active Script");
		if (ImGui::Button("Execute"))
		{
			m_LiveValuesX.Clear();
			m_LiveValuesCH1.Clear();
			m_LiveValuesCH2.Clear();
			m_LiveValuesCH3.Clear();

			m_NodeEditor->ClearScopeBuffers();

			m_NodeEditor->ExecutePostProcessScript();
		}

		ImGui::PopFont();	// OpenSans
	}
	else {
		processDataOpen = false;
	}
	ImGui::PopFont();
	ImGui::EndChild();


	ImGui::EndChild();
	ImGui::End();
	//ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);

	// MenuBar
	ImGui::SetNextWindowPos(menuBarPos);
	ImGui::SetNextWindowSize(menuBarSize);	
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 5.0f));
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Begin("MenuBar", 0, windowFlags);
	ImVec2 childSize = { menuBarSize.x - 10.0f, menuBarSize.y - 2.0f * 5.0f };
	ImGui::BeginChild("Child", childSize, true);

	ImGui::Dummy(ImVec2(2, 0));
	ImGui::SameLine();
	ImGui::PushFont(UICore::GetFont(Font::BigIcons));
	if (ImGui::Button(ICON_MD_START))
	{
		m_LiveValuesX.Clear();
		m_LiveValuesCH1.Clear();
		m_LiveValuesCH2.Clear();
		m_LiveValuesCH3.Clear();

		m_NodeEditor->ClearScopeBuffers();
		m_NodeEditor->ProhibitDeletions();

		m_SerialPort.ClearQueue();
		char buf = 1;
		m_SerialPort.Write(&buf, 1);

		m_PlotWindow.Clear();
		m_PlotWindow2.Clear();

		m_Reading = true;
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_MD_STOP))
	{
		m_SerialPort.ClearQueue();
		char buf = 0;
		m_SerialPort.Write(&buf, 1);

		m_NodeEditor->AllowDeletions();

		m_Reading = false;
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_MD_CLEAR))
	{
		m_LiveValuesX.Clear();
		m_LiveValuesCH1.Clear();
		m_LiveValuesCH2.Clear();
		m_LiveValuesCH3.Clear();

		m_NodeEditor->ClearScopeBuffers();
	}
	ImGui::PopFont();

	ImGui::EndChild();
	ImGui::End();
	ImGui::PopStyleVar(3);
	//ImGui::PopStyleColor();

	ImGui::SetNextWindowPos(dockspacePos);
	ImGui::SetNextWindowSize(dockspaceSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Begin("DockSpace", 0, windowFlags);

	ImVec2 childSize2 = ImVec2{ dockspaceSize.x - 10.0f, dockspaceSize.y - 10.0f };
	ImVec2 dockspaceSize2 = ImVec2{ dockspaceSize.x - 15.0f, dockspaceSize.y - 15.0f };
	ImGui::BeginChild("Child", childSize2, true);
	ImGuiID dockSpaceID = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockSpaceID, childSize2, ImGuiDockNodeFlags_None);
	ImGui::EndChild();

	ImGui::PopStyleVar(3);
	//ImGui::PopStyleColor();
}

void Application::EndDockspace()
{
	ImGui::End();	
}