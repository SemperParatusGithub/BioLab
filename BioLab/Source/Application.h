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

#include "Util/FileUtils.h"

#include "UI/PlotWindow.h"


#define ARDUINO_PORT "\\\\.\\COM7"


class Application
{
public:
	Application();
	~Application();

public:
	void Run();

private:
	void ReadSerialPort();
	void SimulateReadSerialPort();

	void BeginDockspace();
	void EndDockspace();

private:
	static Application* s_Instance;

	std::queue<Vector4f> m_InputQueue;
	std::mutex m_InputQueueMutex;

	SerialPort m_SerialPort;
	std::thread m_ReaderThread;
	std::atomic<bool> m_SerialThreadRunning = true;

	LiveBuffer<float> m_LiveValuesX;
	LiveBuffer<float> m_LiveValuesCH1, m_LiveValuesCH2, m_LiveValuesCH3;

	ImFont* m_BigIcons;

	std::unique_ptr<NodeEditor> m_NodeEditor;
	std::vector<int> m_ActiveScopes;

	bool m_Reading = false;

	std::vector<Signal> m_LoadedSignals;
};