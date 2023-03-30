#include "Application.h"

#include "UI/UICore.h"
#include "UI/IconsMaterialDesign.h"

#include "NodeEditor/Nodes/LiveNodes.h"

#include "Util/FileUtils.h"

#include "SignalProcessing/DigitalFilter.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>


typedef struct
{
	uint32_t sampleIndex;			// Number of sample
	uint16_t ch0, ch1, ch2;			// Sample values
} DataPackage;


static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

Application* Application::s_Instance = nullptr;

Application::Application()
{
	LOG_INFO("creating Application");

	s_Instance = this;

	UICore::Initialize();
	UICore::LoadFonts();
	UICore::SetupStyle();
	UICore::SetLightColorTheme();
	//UICore::SetDarkColorTheme();

	m_NodeEditor = std::make_unique<NodeEditor>();
	m_NodeEditor->SetupStyle();
	m_NodeEditor->SetLightColorTheme();
	//m_NodeEditor->SetDarkColorTheme();

	//m_ReaderThread = std::thread(&Application::SimulateReadSerialPort, this);
	m_ReaderThread = std::thread(&Application::ReadSerialPort, this);

	m_LiveWindow.Close();
	m_GoldbergerWindow.Close();
	m_ECGAnalyzeWindow.Close();

	ImGui::GetIO().IniFilename = nullptr;
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

				m_xValues.PushBack(sample.x);
				m_LiveWindow.AddNewSample(sample);
				m_GoldbergerWindow.AddNewSample(sample);

				m_NodeEditor->AddNewSample(sample);
			
				m_InputQueue.pop();
			}
			m_InputQueueMutex.unlock();
		}

		BeginDockspace();

		m_ECGAnalyzeWindow.Render();
		m_LiveWindow.Render();
		m_GoldbergerWindow.Render();

		m_NodeEditor->Render();	

		for (auto& plotWindow : m_PlotWindows)
			plotWindow.Render();

		// ImGui::ShowDemoWindow();
		// ImPlot::ShowDemoWindow();

		EndDockspace();

		UICore::EndFrame();
		UICore::PollEvents();
	}
}

void Application::ReadSerialPort()
{
	LOG_INFO("Starting Serial Reader thread");

	while (!m_SerialPort.IsConnected() && m_SerialThreadRunning)
	{
		for (auto& serialPort : s_SerialPorts)
		{
			m_SerialPort.Open(serialPort.c_str(), 1000000);
			if (m_SerialPort.IsConnected())
			{
				LOG_INFO("Connected to Serial Port: %s", serialPort.c_str());
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}

	m_SerialPort.ClearQueue();

	LOG_INFO("Ready to receive data");

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

				float xVal = package.sampleIndex / 2000.0f;
				m_RecordingData.CH1.xValues.push_back(xVal);
				m_RecordingData.CH2.xValues.push_back(xVal);
				m_RecordingData.CH3.xValues.push_back(xVal);
				m_RecordingData.aVR.xValues.push_back(xVal);
				m_RecordingData.aVL.xValues.push_back(xVal);
				m_RecordingData.aVF.xValues.push_back(xVal);	
				
				m_RecordingData.CH1.yValues.push_back(package.ch0);
				m_RecordingData.CH2.yValues.push_back(package.ch1);
				m_RecordingData.CH3.yValues.push_back(package.ch2);
				m_RecordingData.aVR.yValues.push_back(-(package.ch0 + package.ch1) / 2.0f);
				m_RecordingData.aVL.yValues.push_back(+(package.ch0 - package.ch2) / 2.0f);
				m_RecordingData.aVF.yValues.push_back(+(package.ch1 + package.ch2) / 2.0f);

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


		float xVal = sampleIndex / 2000.0f;

		m_RecordingData.CH1.xValues.push_back(xVal);
		m_RecordingData.CH2.xValues.push_back(xVal);
		m_RecordingData.CH3.xValues.push_back(xVal);
		m_RecordingData.aVR.xValues.push_back(xVal);
		m_RecordingData.aVL.xValues.push_back(xVal);
		m_RecordingData.aVF.xValues.push_back(xVal);

		m_RecordingData.CH1.yValues.push_back(sample.y);
		m_RecordingData.CH2.yValues.push_back(sample.z);
		m_RecordingData.CH3.yValues.push_back(sample.w);
		m_RecordingData.aVR.yValues.push_back(-(sample.y + sample.z) / 2.0f);
		m_RecordingData.aVL.yValues.push_back(+(sample.y - sample.w) / 2.0f);
		m_RecordingData.aVF.yValues.push_back(+(sample.z + sample.w) / 2.0f);

		{
			const std::lock_guard<std::mutex> lock(m_InputQueueMutex);
			m_InputQueue.push(sample);
			// Lock guard is released when going out of scope
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

void Application::SaveRecordingData()
{
	m_LoadedSignals.push_back(m_RecordingData.CH1);
	m_LoadedSignals.push_back(m_RecordingData.CH2);
	m_LoadedSignals.push_back(m_RecordingData.CH3);
	m_LoadedSignals.push_back(m_RecordingData.aVR);
	m_LoadedSignals.push_back(m_RecordingData.aVL);
	m_LoadedSignals.push_back(m_RecordingData.aVF);

	std::thread recordingThread([&]()
		{
			LOG_INFO("Saving recording data on thread: %d", std::this_thread::get_id());
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			FileUtils::SaveCSV(m_RecordingData.CH1, m_RecordingData.CH1.label + ".csv");
			FileUtils::SaveCSV(m_RecordingData.CH2, m_RecordingData.CH2.label + ".csv");
			FileUtils::SaveCSV(m_RecordingData.CH3, m_RecordingData.CH3.label + ".csv");
			FileUtils::SaveCSV(m_RecordingData.aVR, m_RecordingData.aVR.label + ".csv");
			FileUtils::SaveCSV(m_RecordingData.aVL, m_RecordingData.aVL.label + ".csv");
			FileUtils::SaveCSV(m_RecordingData.aVF, m_RecordingData.aVF.label + ".csv");

			m_RecordingData.CH1 = Signal();
			m_RecordingData.CH2 = Signal();
			m_RecordingData.CH3 = Signal();
			m_RecordingData.aVR = Signal();
			m_RecordingData.aVL = Signal();
			m_RecordingData.aVF = Signal();

			LOG_INFO("Saving recording data on thread FINISHED: %d", std::this_thread::get_id());
		});

	recordingThread.detach();
}

void Application::BeginDockspace()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse  |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport *viewport = ImGui::GetMainViewport();

	ImVec2 sideBarPos = viewport->Pos;
	ImVec2 sideBarSize = ImVec2 { 1.0f / 5.0f * viewport->Size.x, viewport->Size.y };

	// Left window
	ImGui::SetNextWindowPos(sideBarPos);
	ImGui::SetNextWindowSize(sideBarSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowSizeConstraints(ImVec2(100, -1.f), ImVec2(500, -1.f));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 5.0f));
	ImGui::Begin("LeftBar", 0, windowFlags);
	ImGui::SetWindowSize(ImVec2(ImGui::GetWindowSize().x, sideBarSize.y));
	sideBarSize.x = ImGui::GetWindowSize().x;
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
		//ImGui::Text("LiveBuffer Size: %d", m_LiveValuesX.Size());
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
				m_NodeEditor->SaveLiveScript(UICore::SaveFileDialog("Script(*.script)\0*.script\0"));
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Save Script");
			ImGui::SameLine();

			if (ImGui::Button(ICON_MD_FILE_OPEN, ImVec2(30, 23)))
				m_NodeEditor->LoadLiveScript(UICore::OpenFileDialog("Script(*.script)\0*.script\0"));
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

				m_RecordingData.CH1 = FileUtils::CreateSignal("CH1_" + std::string(buffer));
				m_RecordingData.CH2 = FileUtils::CreateSignal("CH2_" + std::string(buffer));
				m_RecordingData.CH3 = FileUtils::CreateSignal("CH3_" + std::string(buffer));
				m_RecordingData.aVR = FileUtils::CreateSignal("aVR_" + std::string(buffer));
				m_RecordingData.aVL = FileUtils::CreateSignal("aVL_" + std::string(buffer));
				m_RecordingData.aVF = FileUtils::CreateSignal("aVF_" + std::string(buffer));
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Start Recording");
			ImGui::SameLine();
			if (ImGui::Button(ICON_MD_STOP, ImVec2(30, 23)) || elapsed > m_RecordingDuration && m_Recording)
			{
				m_Recording = false;

				SaveRecordingData();
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
	if (ImGui::CollapsingHeader("Offline Analysis"))
	{
		processDataOpen = true;
		ImGui::PushFont(UICore::GetFont(Font::OpenSans));

		ImGui::Separator();
		ImGui::PushFont(UICore::GetFont(Font::OpenSansHeading));
		ImGui::TextUnformatted("Loaded Signals");
		ImGui::PopFont();
		ImGui::Separator();

		if (ImGui::BeginChild("LoadedSignals + Options", ImVec2(65.0f, 180.0f)))
		{
			if (ImGui::Button("Load", ImVec2(-1, 0)))
			{
				std::string path = UICore::OpenFileDialog("Data file (*.csv)");
				auto& signal = FileUtils::LoadCSV(path);
				if(signal.id != -1)
					m_LoadedSignals.push_back(signal);
			}
			if (ImGui::Button("Clear", ImVec2(-1, 0)))
			{
				m_LoadedSignals.clear();
			}
			ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild("Signals", ImVec2(0.0f, 180.0f), true);
			for (int i = 0; i < m_LoadedSignals.size(); i++)
			{
				auto& signal = m_LoadedSignals[i];
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
				std::string openPopupName = "Options##" + std::to_string(signal.id);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup(openPopupName.c_str());
				}
				if (ImGui::BeginPopup(openPopupName.c_str()))
				{
					if (ImGui::MenuItem("QRS Detection"))
					{
						//m_LoadedSignals.push_back(QRSDetection(signal));
						m_ECGAnalyzeWindow.ProcessSignal(signal);
					}
					if (ImGui::MenuItem(ICON_MD_DELETE"  Delete"))
					{
						m_LoadedSignals.erase(m_LoadedSignals.begin() + i);
					}
					ImGui::EndPopup();
				}
				else if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					ImGui::BeginTooltip();
					ImGui::Text("ID: %d", signal.id);
					ImGui::Text("Size: %d", signal.xValues.size());
					ImGui::Text("Stride: %d", signal.stride);
					ImGui::EndTooltip();
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
			m_NodeEditor->SaveLiveScript(UICore::SaveFileDialog("Script(*.script)\0*.script\0"));
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Save Script");
		ImGui::SameLine();

		if (ImGui::Button(ICON_MD_FILE_OPEN, ImVec2(30, 23)))
			m_NodeEditor->LoadLiveScript(UICore::OpenFileDialog("Script(*.script)\0*.script\0"));
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
			m_xValues.Clear();
			m_LiveWindow.ClearBuffers();
			m_GoldbergerWindow.ClearBuffers();
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


	ImVec2 menuBarPos = ImVec2{ viewport->Pos.x + sideBarSize.x, viewport->Pos.y };
	ImVec2 menuBarSize = ImVec2{ viewport->Size.x - sideBarSize.x, 55.0f };

	ImVec2 dockspacePos = ImVec2{ viewport->Pos.x + sideBarSize.x, viewport->Pos.y + menuBarSize.y };
	ImVec2 dockspaceSize = ImVec2{ viewport->Size.x - sideBarSize.x, viewport->Size.y - menuBarSize.y };

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
		m_LiveWindow.Open();

		m_LiveWindow.ClearBuffers();
		m_GoldbergerWindow.ClearBuffers();
		m_xValues.Clear();

		for (auto& plotWindow : m_PlotWindows)
			plotWindow.Clear();

		m_NodeEditor->ClearScopeBuffers();
		m_NodeEditor->ProhibitDeletions();

		m_SerialPort.ClearQueue();
		char buf = 1;
		m_SerialPort.Write(&buf, 1);

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
		m_xValues.Clear();
		m_GoldbergerWindow.ClearBuffers();
		m_LiveWindow.ClearBuffers();

		m_NodeEditor->ClearScopeBuffers();
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_MD_ADD_TO_QUEUE))
	{
		auto& newWindow = m_PlotWindows.emplace_back();
		newWindow.SetName("Plot Window " + std::to_string(FileUtils::GetNextSignalID()));
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_MD_PERSONAL_VIDEO))
	{
		m_GoldbergerWindow.Open();
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