#pragma once

#include "Core.h"
#include <unordered_map>


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

	static u64 LoadTexture(const std::string& filepath);

	static std::string OpenFileDialog(const char* filter);
	static std::string SaveFileDialog(const char* filter);

	static void LoadFonts();
	static ImFont* GetFont(Font font);

	static void SetupStyle();
	static void SetLightColorTheme();

private:
	static std::unordered_map<int, ImFont*> s_Fonts;
};