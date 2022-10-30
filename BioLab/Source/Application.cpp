#include "Application.h"

#include "Core.h"

#include <imgui.h>
#include <implot.h>

#include <GLFW/glfw3.h>

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
	m_Window->Maximize();

	//const auto* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	//m_Window->SetPosition(Vector2f{ vidmode->width / 2.0f - 640.0f, vidmode->height / 2.0f - 360.0f });

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

	float xData[1000], yData1[1000], yData2[1000];
	m_LiveValuesX.push_back(-0.1f);
	m_LiveValuesCH1.push_back(0.0f);
	m_LiveValuesCH2.push_back(0.0f);
	m_LiveValuesCH3.push_back(0.0f);

	for (int i = 0; i < 1000; i++)
	{
		xData[i] = (float)i;
		yData1[i] = sin(2.0f * 3.14159f * 0.01f * (float)i);
		yData2[i] = cos(2.0f * 3.14159f * 0.01f * (float)i);
	}

	while (m_Window->isOpen())
	{
		m_Window->BeginFrame();

		m_Window->Update();
		m_Window->Clear();

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
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::Begin("Test Window", (bool*)0, windowFlags);

			ImGui::PopStyleVar(1);

			auto avail = ImGui::GetContentRegionAvail();
			auto spacing = ImGui::GetStyle().ItemSpacing;
			ImGui::BeginChild("Control:Child", ImVec2(avail.x / 4, -1.0f), true);
			ImGui::Text("[Controls]");
			ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
			ImGui::Spacing();
			ImGui::Text("[Buffer sizes]");
			ImGui::Text("X:   %d", m_LiveValuesX.size());
			ImGui::Text("CH1: %d", m_LiveValuesCH1.size());
			ImGui::Text("CH3: %d", m_LiveValuesCH2.size());
			ImGui::Text("CH4: %d", m_LiveValuesCH3.size());
			m_InputQueueMutex.lock();
			ImGui::Text("queue: %d", m_InputQueue.size());
			m_InputQueueMutex.unlock();
			ImGui::EndChild();

			ImGui::SameLine();
			
			ImVec2 graphSize = ImVec2(avail.x * 3 / 4 - spacing.x, -1.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::BeginChild("Graph:Child", graphSize, false);
			// plot data
			{
				ImPlot::SetNextAxisLimits(ImAxis_Y1, -2000, 6500, ImGuiCond_FirstUseEver);\
				ImPlot::SetNextAxisLimits(ImAxis_X1, (float)m_LiveValuesX.back() - 9000.0f, (float)m_LiveValuesX.back() + 1000.0f, ImGuiCond_Always);\

				ImPlot::BeginPlot("##plot", 0, 0, graphSize);

				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("CH1", m_LiveValuesX.data(), m_LiveValuesCH1.data(), m_LiveValuesX.size());	

				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("CH2", m_LiveValuesX.data(), m_LiveValuesCH2.data(), m_LiveValuesX.size());	

				ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
				ImPlot::PlotLine("CH3", m_LiveValuesX.data(), m_LiveValuesCH3.data(), m_LiveValuesX.size());

				ImPlot::EndPlot();

				ImGui::PopStyleVar();
			}
			ImGui::EndChild();

			ImGui::End();
		}

		m_Window->EndFrame();
	}
}

void Application::ReadSerialPort()
{
	std::cout << "Starting Serial Reader thread" << std::endl;

	m_SerialPort.Open(ARDUINO_PORT, 1000000);
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

				if (idx <= 10)
				{
					idx++;
					continue;
				}

				idx = 0;
				Vector4f sample = { package.sampleIndex, 
					package.ch0, package.ch1, package.ch2 };

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