#pragma once


struct GLFWwindow;

class UICore
{
public:
	static void Initialize();
	static void Shutdown();

	static bool WindowOpen();

	static void BeginFrame();
	static void EndFrame();

	static void PollEvents();
};