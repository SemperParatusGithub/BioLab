#pragma once
#include <iostream>
#include <Windows.h>

// Code mainly taken from: https://playground.arduino.cc/Interfacing/CPPWindows/

#define ARDUINO_WAIT_TIME 1000


class SerialPort
{
public:
	SerialPort();
	~SerialPort();

	void Open(const char *portName, DWORD baudRate);
	void Close();

	std::size_t InQueue();
	void ClearQueue();

	bool IsConnected() const;

	bool Read(void *buffer, std::size_t buf_size);
	bool Write(void*buffer, std::size_t buf_size);

private:
	HANDLE m_Handle;
	bool m_IsConnected = false;
	COMSTAT m_Status;
	DWORD m_Errors;
};