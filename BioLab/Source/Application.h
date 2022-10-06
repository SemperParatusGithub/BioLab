#pragma once

#include "UI/Window.h"


class Application
{
public:
	Application();
	~Application();

public:
	void Run();

private:
	std::unique_ptr<Window> m_Window;
};