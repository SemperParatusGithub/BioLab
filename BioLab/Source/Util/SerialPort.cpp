#include "SerialPort.h"


SerialPort::SerialPort()
{
}
SerialPort::~SerialPort()
{
	if (m_IsConnected) {
		m_IsConnected = false;
		CloseHandle(m_Handle);
	}
}

void SerialPort::Open(const char *portName, DWORD baudRate)
{
	printf("Opening serial port: %s\n", portName);
	m_IsConnected = false;

	m_Handle = CreateFileA(static_cast<LPCSTR>(portName),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (m_Handle == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			printf("[ERROR] %s not available\n", portName);
		}
		else
		{
			printf("[ERROR] Failed to open serial port\n");
		}
	}
	else {
		DCB dcbSerialParameters = { 0 };

		if (!GetCommState(m_Handle, &dcbSerialParameters)) {
			printf("[ERROR]: failed to get current serial parameters\n");
		}
		else {
			printf("Successfully connected to serial port: %s\n", portName);
			dcbSerialParameters.BaudRate = baudRate;
			dcbSerialParameters.ByteSize = 8;
			dcbSerialParameters.StopBits = ONESTOPBIT;
			dcbSerialParameters.Parity = NOPARITY;
			dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

			if (!SetCommState(m_Handle, &dcbSerialParameters))
			{
				printf("[WARNING]: could not set Serial port parameters\n");
			}
			else {
				m_IsConnected = true;
				PurgeComm(m_Handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
				Sleep(ARDUINO_WAIT_TIME);
			}
		}
	}
}
void SerialPort::Close()
{
	if (m_IsConnected) {
		m_IsConnected = false;
		CloseHandle(m_Handle);
	}
}

std::size_t SerialPort::InQueue()
{
	ClearCommError(m_Handle, &m_Errors, &m_Status);
	return m_Status.cbInQue;
}
void SerialPort::ClearQueue()
{
	PurgeComm(m_Handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
}


bool SerialPort::IsConnected() const
{
	return m_IsConnected;
}

bool SerialPort::Read(void *buffer, std::size_t buf_size)
{
	while (InQueue() < buf_size);

	DWORD bytesRead;
	unsigned int toRead = buf_size;

	if (ReadFile(m_Handle, buffer, toRead, &bytesRead, NULL)) return true;

	return 0;
}


bool SerialPort::Write(void *buffer, std::size_t buf_size)
{
	DWORD bytesSend;

	if (!WriteFile(m_Handle, (void *)buffer, buf_size, &bytesSend, 0)) {
		ClearCommError(m_Handle, &m_Errors, &m_Status);
		return false;
	}
	else return true;
}