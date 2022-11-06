#pragma once


struct GLFWwindow;

class UICore
{
public:
	static void Initialize(GLFWwindow* window, const char* glslVersion);
	static void Shutdown();

	static void BeginFrame();
	static void EndFrame();

	static void SetStyle();

	static void SetLightColorTheme();
	static void SetDarkColorTheme();
};