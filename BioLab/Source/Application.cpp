#include "Application.h"

#include "Core.h"

#include <imgui.h>
#include <implot.h>



Application::Application()
{
	LOG_INFO("creating Application");

	m_Window = std::make_unique<Window>("BioLab", 1280, 720);
	m_Window->Maximize();

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

	float xData[1000], yData1[1000], yData2[1000];
	m_LiveValuesX.push_back(-0.1f);
	m_LiveValuesCH1.push_back(0.0f);
	m_LiveValuesCH2.push_back(0.0f);

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
			
				m_InputQueue.pop();
			}
			m_InputQueueMutex.unlock();
		}

		m_InputQueueMutex.lock();
		if (m_LiveValuesX.size() >= 10000)
		{
			m_LiveValuesX.erase(m_LiveValuesX.begin(), m_LiveValuesX.begin() + 250);
			m_LiveValuesCH1.erase(m_LiveValuesCH1.begin(), m_LiveValuesCH1.begin() + 250);
			m_LiveValuesCH2.erase(m_LiveValuesCH2.begin(), m_LiveValuesCH2.begin() + 250);
		}
		m_InputQueueMutex.unlock();

		ImGui::Begin("Test Window");
		ImGui::Text("Hello, World!");
		static float var = 0.0f;
		ImGui::SliderFloat("value", &var, 0.0f, 100.0f);
		ImGui::Text("%.2f", (float)m_LiveValuesX[0] - 40.f);
		ImGui::Text("%.2f", (float)m_LiveValuesX.back() + 40.f);
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
		ImGui::End();

		ImGui::Begin("Graph");
		ImPlot::SetNextAxisLimits(ImAxis_Y1, -2, 2, ImGuiCond_FirstUseEver);
		ImPlot::SetNextAxisLimits(ImAxis_X1, (float)m_LiveValuesX.back() - 9750, (float)m_LiveValuesX.back() + 250.f, ImGuiCond_Always);
		if (ImPlot::BeginPlot("##Digital", 0, 0, ImGui::GetContentRegionAvail())) 
		{
			ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
			ImPlot::PlotLine("CH1", m_LiveValuesX.data(), m_LiveValuesCH1.data(), m_LiveValuesX.size());
			ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, -1), 2.0f);
			ImPlot::PlotLine("CH2", m_LiveValuesX.data(), m_LiveValuesCH2.data(), m_LiveValuesX.size());
			ImPlot::EndPlot();
		}
		ImGui::End();

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
			Vector3f sample;
			if (m_SerialPort.Read((void*)&sample, 12))
			{
				//std::cout <<
				//	sample.x << ", " <<
				//	sample.y << ", " <<
				//	sample.z << "\n";

				samplesReceived++;

				if (idx <= 10)
				{
					idx++;
					continue;
				}

				idx = 0;

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
