#pragma once

#include "UI/Window.h"
#include "SerialPort.h"

#include <queue>
#include <thread>
#include <atomic>
#include <mutex>


#define ARDUINO_PORT "\\\\.\\COM4"


class Application
{
public:
	Application();
	~Application();

public:
	void Run();

private:
	void ReadSerialPort();

private:
	std::queue<Vector3f> m_InputQueue;
	std::mutex m_InputQueueMutex;

	std::unique_ptr<Window> m_Window;
	SerialPort m_SerialPort;
	std::thread m_ReaderThread;
	std::atomic<bool> m_SerialThreadRunning = true;

	std::vector<float> m_LiveValuesX;
	std::vector<float> m_LiveValuesCH1, m_LiveValuesCH2;
};