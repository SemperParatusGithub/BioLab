#pragma once


class Application
{
public:
	Application();
	~Application();

public:
	void Run();

private:
	bool m_Running = false;
};