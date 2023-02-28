#include "Application.h"

#include "UI/UICore.h"
#include "UI/IconsMaterialDesign.h"

#include "NodeEditor/Nodes/Scope.h"

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

	m_ReaderThread = std::thread(&Application::SimulateReadSerialPort, this);
	//m_ReaderThread = std::thread(&Application::ReadSerialPort, this);
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
						const ImGuiPayload* scopePayload = ImGui::AcceptDragDropPayload("DND_SCOPE");
						if (scopePayload)
						{
							int id = *(int*)scopePayload->Data;
							LOG_INFO("dropped Scope: %d", id);

							if (std::find(m_ActiveScopes.begin(), m_ActiveScopes.end(), id) == m_ActiveScopes.end())
								m_ActiveScopes.push_back(id);
						}

						const ImGuiPayload* signalPayload = ImGui::AcceptDragDropPayload("DND_SIGNAL");
						if (signalPayload)
						{
							int id = *(int*)signalPayload->Data;
							LOG_INFO("dropped Signal: %d", id);

							if (std::find(m_ActiveSignals.begin(), m_ActiveSignals.end(), id) == m_ActiveSignals.end())
								m_ActiveSignals.push_back(id);
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
								ImPlot::PlotLine(scopeName.c_str(), m_LiveValuesX.Data(), scopeNode->Samples.Data(), scopeNode->Samples.Size());
						}
					}
					for (auto& id : m_ActiveSignals)
					{
						for (auto& signal : m_LoadedSignals)
						{
							if (signal.id == id)
							{
								ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, signal.alpha);

								ImPlot::SetNextLineStyle(signal.color, signal.thickness);
								if (signal.markers) ImPlot::SetNextMarkerStyle(ImPlotMarker_Square);
								if(signal.shaded)
									ImPlot::PlotShaded(signal.label.c_str(), signal.xValues.data(), signal.yValues.data(), signal.xValues.size());
								else
									ImPlot::PlotLine(signal.label.c_str(), signal.xValues.data(), signal.yValues.data(), signal.xValues.size());
							
								ImPlot::PopStyleVar();
							}
						}
					}

					ImPlot::EndPlot();
				}
				ImGui::End();
			}

			m_NodeEditor->Render();			
			m_NodeEditor->ShowDebugWindow();
			//
			//ImGui::ShowDemoWindow();
			ImPlot::ShowDemoWindow();


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
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Begin("LeftBar", 0, windowFlags);
	ImGui::BeginChild("Child", ImVec2(-1, -1), true);

	ImGui::PushFont(UICore::GetFont(Font::OpenSansHeading));
	ImGui::TextUnformatted(ICON_MD_LOGO_DEV"  BioLab");
	ImGui::PopFont();

	float width = ImGui::GetContentRegionAvail().x;


	static bool statisticsOpen = false;
	ImGui::BeginChild("Statistics", ImVec2(width, statisticsOpen ? 150 : 40), true);
	if (ImGui::CollapsingHeader("Statistics"))
	{
		statisticsOpen = true;
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
		ImGui::Text("Mouse Position: %.2f, %.2f", ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		ImGui::Text("LiveBuffer Size: %d", m_LiveValuesX.Size());
	}
	else {
		statisticsOpen = false;
	}
	ImGui::EndChild();

	static bool liveDataOpen = false;
	ImGui::BeginChild("LiveDataChild", ImVec2(width, liveDataOpen ? 395.0f : 40), true);
	{
		//ImGui::PushFont(UICore::GetFont(Font::OpenSansHeading));
		if (ImGui::CollapsingHeader("Live Data"))
		{
			liveDataOpen = true;
			//ImGui::PopFont();

			static int amplification[3] = { 100, 100, 100 };
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
			ImGui::SliderInt("CH1", &amplification[0], 0, 255);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
			ImGui::SliderInt("CH2", &amplification[1], 0, 255);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
			ImGui::SliderInt("CH3", &amplification[2], 0, 255);

			if (ImGui::Button(ICON_MD_SEND, ImVec2(30.0f, 25.0f)))
			{
				LOG_INFO("Sending Values: [%d, %d, %d]", amplification[0], amplification[1], amplification[2]);

				unsigned char MAG_BYTE = 121;
				unsigned char writeData[4] = { MAG_BYTE, amplification[0], amplification[1], amplification[2] };

				// m_SerialPort.Write uses WriteFile under the hood which is thread safe so there is no problem!!!
				m_SerialPort.Write((void *)writeData, 4);
			}

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
			if (ImGui::Button("Save"))
			{
				m_NodeEditor->SaveLiveScript(UICore::SaveFileDialog("Live Script(*.livescript)\0*.livescript\0"));
			}
			ImGui::SameLine();
			if (ImGui::Button("Load"))
			{
				m_NodeEditor->LoadLiveScript(UICore::OpenFileDialog("Live Script(*.livescript)\0*.livescript\0"));
			}
			ImGui::SameLine();
			if (ImGui::Button("Create"))
			{
				m_NodeEditor->CreateLiveScript();
			}
			ImGui::PopStyleColor();

			auto dif = std::chrono::high_resolution_clock::now() - m_RecordingStart;
			float elapsed = m_Recording ? dif.count() / 1e9 : 0.0f;

			if (ImGui::Button("Start Recording"))
			{
				m_Recording = true;
				m_RecordingStart = std::chrono::high_resolution_clock::now();

				std::time_t now = std::time(nullptr);  // get current time
				char buffer[80];
				std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", std::localtime(&now));
				std::cout << "Current date and time: " << buffer << std::endl;
				m_OutputSignalCH1.label = "CH1  " + std::string(buffer);
				m_OutputSignalCH2.label = "CH2  " + std::string(buffer);
				m_OutputSignalCH3.label = "CH3  " + std::string(buffer);
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop Recording") || elapsed > m_RecordingDuration && m_Recording)
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
			ImGui::SliderFloat("Druation", &m_RecordingDuration, 5.0f, 20.0f);
			ImGui::Text("Recording: %.2f", elapsed);
			ImGui::ProgressBar(elapsed / m_RecordingDuration, ImVec2(ImGui::GetContentRegionAvail().x, 75.0f));
		}
		else {
			liveDataOpen = false;
		}
	}
	ImGui::EndChild();

	static bool processDataOpen = false;
	ImGui::BeginChild("ProcessDataChild", ImVec2(width, processDataOpen ? width : 40), true);
	if (ImGui::CollapsingHeader("Data Analysis"))
	{
		processDataOpen = true;
		{
			ImGui::BeginChild("SignalOptions", ImVec2(width * 0.25f, width * 0.5f));
			if (ImGui::Button("Load", ImVec2(-1, 0)))
			{
				std::string path = UICore::OpenFileDialog("Data file (*.csv)");
				auto &signal = FileUtils::LoadCSV(path);
				m_LoadedSignals.push_back(signal);
			}
			if (ImGui::Button("Clear", ImVec2(-1, 0)))
			{
				m_LoadedSignals.clear();
			}
			ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild("Signals", ImVec2(0.0f, width * 0.5f), true);
			for (auto &signal : m_LoadedSignals)
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

		if (m_NodeEditor->GetActiveScript().GetScriptType() == Script::Type::PostProcessScript)
			ImGui::Text("Active Script: %s", m_NodeEditor->GetActiveScript().GetName().c_str());
		else
			ImGui::Text("No active postprocess Script");

		ImGui::SameLine();
		ImGui::Button(ICON_MD_FILE_OPEN, ImVec2(30.0f, 25.0f));

		if (ImGui::Button("Save"))
		{
			m_NodeEditor->SaveLiveScript(UICore::SaveFileDialog("Post Process(*.postprocess)\0*.postprocess\0"));
		}
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			m_NodeEditor->LoadLiveScript(UICore::OpenFileDialog("Post Process(*.postprocess)\0*.postprocess\0"));
		}
		ImGui::SameLine();
		if (ImGui::Button("Create"))
		{
			m_NodeEditor->CreatePostProcessScript();
		}
		if (ImGui::Button("Execute"))
		{
			m_LiveValuesX.Clear();
			m_LiveValuesCH1.Clear();
			m_LiveValuesCH2.Clear();
			m_LiveValuesCH3.Clear();

			m_NodeEditor->ClearScopeBuffers();

			m_NodeEditor->ExecutePostProcessScript();
		}
	}
	else {
		processDataOpen = false;
	}
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