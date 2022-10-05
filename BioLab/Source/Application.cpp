#include "Application.h"

#include <iostream>


Application::Application()
{
	std::cout << "creating Application" << std::endl;

	m_Running = true;
}

Application::~Application()
{
	std::cout << "shutting down Application" << std::endl;
}

void Application::Run()
{
	std::cout << "running Application" << std::endl;
}