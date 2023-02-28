#pragma once

#include "Util/SerialPort.h"
#include "Util/LiveBuffer.h"

#include "NodeEditor/NodeEditor.h"

#include <imgui.h>
#include <implot.h>

#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

#include "Util/FileUtils.h"

#include "UI/PlotWindow.h"


#define ARDUINO_PORT "\\\\.\\COM3"


class Application
{
public:
	Application();
	~Application();

	static Application* Instance() { return s_Instance; }
	Signal GetSignalByID(int signalID)
	{
		for (auto& signal : m_LoadedSignals)
		{
			if (signal.id == signalID)
				return signal;
		}
		return Signal();
	}

public:
	void Run();

private:
	void ReadSerialPort();
	void SimulateReadSerialPort();

	void BeginDockspace();
	void EndDockspace();

private:
	friend class NodeEditor;

	static Application* s_Instance;

	std::queue<Vector4f> m_InputQueue;
	std::mutex m_InputQueueMutex;

	SerialPort m_SerialPort;
	std::thread m_ReaderThread;
	std::atomic<bool> m_SerialThreadRunning = true;

	std::atomic<bool> m_Recording = false;
	Signal m_OutputSignalCH1;
	Signal m_OutputSignalCH2;
	Signal m_OutputSignalCH3;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_RecordingStart;
	float m_RecordingDuration = 10.0f;

	LiveBuffer<float> m_LiveValuesX;
	LiveBuffer<float> m_LiveValuesCH1, m_LiveValuesCH2, m_LiveValuesCH3;

	std::unique_ptr<NodeEditor> m_NodeEditor;

	std::vector<int> m_ActiveScopes;
	std::vector<int> m_ActiveSignals;

	bool m_Reading = false;

	std::vector<Signal> m_LoadedSignals;
};