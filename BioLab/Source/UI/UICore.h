#pragma once


struct GLFWwindow;

class UICore
{
public:
	static void Initialize(GLFWwindow* window, const char* glslVersion);
	static void Shutdown();

	static void BeginFrame();
	static void EndFrame();
};