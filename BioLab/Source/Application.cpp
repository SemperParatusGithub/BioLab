#include "Application.h"

#include "Core.h"

#include <imgui.h>


Application::Application()
{
	LOG_INFO("creating Application");

	m_Window = std::make_unique<Window>("BioLab", 1280, 720);
	// m_Window->Maximize();
}

Application::~Application()
{
	LOG_INFO("shutting down Application");
}

void Application::Run()
{
	LOG_INFO("running Application");

	while (m_Window->isOpen())
	{
		m_Window->BeginFrame();

		m_Window->Update();
		m_Window->Clear();

		ImGui::Begin("Test Window");
		ImGui::Text("Hello, World!");
		static float var = 0.0f;
		ImGui::SliderFloat("value", &var, 0.0f, 100.0f);
		ImGui::End();

		m_Window->EndFrame();
	}
}