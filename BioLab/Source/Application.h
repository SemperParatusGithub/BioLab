#pragma once

#include "UI/Window.h"
#include "SerialPort.h"

#include "NodeEditor/Node.h"
#include "NodeEditor/Comment.h"

#include "NodeEditor/NodeEditor.h"

#include "Util/LiveBuffer.h"

#include <queue>
#include <thread>
#include <atomic>
#include <mutex>

#include <imgui.h>


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

	void BeginDockspace();
	void EndDockspace();

private:
	std::queue<Vector4f> m_InputQueue;
	std::mutex m_InputQueueMutex;

	SerialPort m_SerialPort;
	std::thread m_ReaderThread;
	std::atomic<bool> m_SerialThreadRunning = true;

	LiveBuffer<float> m_LiveValuesX;
	LiveBuffer<float> m_LiveValuesCH1, m_LiveValuesCH2, m_LiveValuesCH3;

	ImFont* m_BigIcons;

	std::unique_ptr<NodeEditor> m_NodeEditor;
	int m_ActiveScopeID = 0;
};