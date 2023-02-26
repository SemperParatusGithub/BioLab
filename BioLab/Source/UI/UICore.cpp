#include "UICore.h"
#include "Core.h"

#include "IconsMaterialDesign.h"

#include <stb_image.h>

#include <implot.h>
#include <imgui.h>
#include <imgui_node_editor.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>


#if defined(BIOLAB_WIN32_DIRECTX12)
#include "Backends/Win32_DirectX.h"
#elif defined(BIOLAB_GLFW_OPENGL3)
#include "Backends/GLFW_Opengl.h"
#endif


#if defined(BIOLAB_WIN32_DIRECTX12)
namespace Backend = DirectX;
#elif defined(BIOLAB_GLFW_OPENGL3)
namespace Backend = OpenGL;
#endif


std::unordered_map<int, ImFont*> UICore::s_Fonts;

void UICore::Initialize()
{
	Backend::Initialize("TestWindow", 1280, 720);
}
void UICore::Shutdown()
{
	Backend::Shutdown();
}

bool UICore::WindowOpen()
{
	return Backend::WindowOpen();
}

void UICore::BeginFrame()
{
	Backend::BeginFrame();
}
void UICore::EndFrame()
{
	Backend::EndFrame();
}

void UICore::PollEvents()
{
	Backend::PollEvents();
}

u64 UICore::LoadTexture(const std::string& filepath)
{
	LOG_INFO("Loading texture: %s", filepath.c_str());

	int width = 0, height = 0, channels = 0;

	if (stbi_uc* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4))
	{
		auto textureHandle = Backend::CreateTexture(data, width, height);

		return textureHandle;
	}

	LOG_ERROR("Failed to load texture: %s", filepath.c_str());
	return 0;
}

std::string UICore::OpenFileDialog(const char* filter)
{
	return Backend::OpenFileDialog(filter);
}

std::string UICore::SaveFileDialog(const char* filter)
{
	return Backend::SaveFileDialog(filter);
}

void UICore::LoadFonts()
{
	static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 4;
	icons_config.OversampleV = 4;

	ImFontConfig icons_config1;
	icons_config1.OversampleH = 4;
	icons_config1.OversampleV = 4;
	ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/Fonts/OpenSans/OpenSans-Regular.ttf", 20.0f, &icons_config1);
	ImFont* openSans = ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/MaterialIcons-Regular.ttf", 20.0f, &icons_config, icons_ranges);

	ImFont* openSansHeading = ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/Fonts/OpenSans/OpenSans-Bold.ttf", 30.0f);
	ImFont* bigIcons = ImGui::GetIO().Fonts->AddFontFromFileTTF("../../BioLab/Ressources/MaterialIcons-Regular.ttf", 30, 0, icons_ranges);
	
	s_Fonts[(int)Font::Default] = openSans;
	s_Fonts[(int)Font::OpenSans] = openSans;
	s_Fonts[(int)Font::OpenSansHeading] = openSansHeading;
	s_Fonts[(int)Font::BigIcons] = bigIcons;
}

ImFont* UICore::GetFont(Font font)
{
	return s_Fonts[int(font)];
}

void UICore::SetupStyle()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ImGui styling
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ImGuiStyle* imguiStyle = &ImGui::GetStyle();

	imguiStyle->WindowMinSize = ImVec2(10, 10);
	imguiStyle->WindowPadding = ImVec2(10, 10);
	imguiStyle->WindowRounding = 0.0f;
	imguiStyle->FramePadding = ImVec2(5, 5);
	imguiStyle->FrameRounding = 5.0f;
	imguiStyle->ItemSpacing = ImVec2(8, 6);
	imguiStyle->ItemInnerSpacing = ImVec2(6, 4);
	imguiStyle->IndentSpacing = 25.0f;
	imguiStyle->ScrollbarSize = 15.0f;
	imguiStyle->ScrollbarRounding = 0.0f;
	imguiStyle->GrabRounding = 0.0f;
	imguiStyle->TabRounding = 0.0f;
	imguiStyle->ChildRounding = 0.0f;
	imguiStyle->PopupRounding = 0.0f;
	imguiStyle->WindowBorderSize = 1.0f;
	imguiStyle->ChildBorderSize = 1.0f;
	imguiStyle->FrameBorderSize = 0.0f;
	imguiStyle->PopupBorderSize = 1.0f;
	imguiStyle->WindowMenuButtonPosition = ImGuiDir_None;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ImPlot styling
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ImPlotStyle* implotStyle = &ImPlot::GetStyle();

	implotStyle->LineWeight = 1.5;
	implotStyle->Marker = ImPlotMarker_None;
	implotStyle->MarkerSize = 4;
	implotStyle->MarkerWeight = 1;
	implotStyle->FillAlpha = 0.5f;
	implotStyle->ErrorBarSize = 5;
	implotStyle->ErrorBarWeight = 1.5f;
	implotStyle->DigitalBitHeight = 8;
	implotStyle->DigitalBitGap = 4;
	implotStyle->PlotBorderSize = 2;
	implotStyle->MinorAlpha = 1.0f;
	implotStyle->MajorTickLen = ImVec2(10, 10);
	implotStyle->MinorTickLen = ImVec2(5, 5);
	implotStyle->MajorTickSize = ImVec2(2.0f, 2.0f);
	implotStyle->MinorTickSize = ImVec2(2.0f, 2.0f);
	implotStyle->MajorGridSize = ImVec2(1.0f, 1.0f);
	implotStyle->MinorGridSize = ImVec2(1.0f, 1.0f);
	implotStyle->PlotPadding = ImVec2(12, 12);
	implotStyle->LabelPadding = ImVec2(5, 5);
	implotStyle->LegendPadding = ImVec2(5, 5);
	implotStyle->MousePosPadding = ImVec2(5, 5);
	implotStyle->PlotMinSize = ImVec2(300, 225);
}

void UICore::SetLightColorTheme()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ImGui colors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ImGuiStyle* imguiStyle = &ImGui::GetStyle();
	ImVec4* imguiColors = imguiStyle->Colors;

	imguiColors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	imguiColors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	imguiColors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	imguiColors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	imguiColors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
	imguiColors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
	imguiColors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	imguiColors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	imguiColors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	imguiColors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	imguiColors[ImGuiCol_TitleBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	imguiColors[ImGuiCol_TitleBgActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	imguiColors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	imguiColors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	imguiColors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	imguiColors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	imguiColors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	imguiColors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	imguiColors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	imguiColors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	imguiColors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
	imguiColors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	imguiColors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.4f);
	imguiColors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	imguiColors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	imguiColors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	imguiColors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	imguiColors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
	imguiColors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	imguiColors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	imguiColors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
	imguiColors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	imguiColors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	imguiColors[ImGuiCol_Tab] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f); //ImLerp(imguiColors[ImGuiCol_Header], imguiColors[ImGuiCol_TitleBgActive], 0.90f);
	imguiColors[ImGuiCol_TabHovered] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	imguiColors[ImGuiCol_TabActive] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f); // ImLerp(imguiColors[ImGuiCol_HeaderActive], imguiColors[ImGuiCol_TitleBgActive], 0.60f);
	imguiColors[ImGuiCol_TabUnfocused] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f); //ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // ImLerp(imguiColors[ImGuiCol_Tab], imguiColors[ImGuiCol_TitleBg], 0.80f);
	imguiColors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f); //ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // ImLerp(imguiColors[ImGuiCol_TabActive], imguiColors[ImGuiCol_TitleBg], 0.40f);
	imguiColors[ImGuiCol_DockingPreview] = imguiColors[ImGuiCol_Header] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	imguiColors[ImGuiCol_DockingEmptyBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	imguiColors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	imguiColors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	imguiColors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	imguiColors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	imguiColors[ImGuiCol_TableHeaderBg] = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
	imguiColors[ImGuiCol_TableBorderStrong] = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);   // Prefer using Alpha=1.0 here
	imguiColors[ImGuiCol_TableBorderLight] = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);   // Prefer using Alpha=1.0 here
	imguiColors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	imguiColors[ImGuiCol_TableRowBgAlt] = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
	imguiColors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	imguiColors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	imguiColors[ImGuiCol_NavHighlight] = imguiColors[ImGuiCol_HeaderHovered];
	imguiColors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
	imguiColors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
	imguiColors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ImPlot colors
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ImPlotStyle* implotStyle = &ImPlot::GetStyle();
	ImVec4* implotColors = implotStyle->Colors;

	implotColors[ImPlotCol_Line] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_Fill] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_MarkerFill] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_ErrorBar] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	implotColors[ImPlotCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	implotColors[ImPlotCol_PlotBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	implotColors[ImPlotCol_PlotBorder] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	implotColors[ImPlotCol_LegendBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	implotColors[ImPlotCol_LegendBorder] = ImVec4(0.80f, 0.81f, 0.85f, 1.00f);
	implotColors[ImPlotCol_LegendText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	implotColors[ImPlotCol_TitleText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	implotColors[ImPlotCol_InlayText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	implotColors[ImPlotCol_AxisText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	implotColors[ImPlotCol_AxisTick] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	implotColors[ImPlotCol_AxisGrid] = ImVec4(0.80f, 0.81f, 0.85f, 1.00f);
	implotColors[ImPlotCol_AxisBgHovered] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
	implotColors[ImPlotCol_AxisBgActive] = ImVec4(0.92f, 0.92f, 0.95f, 0.75f);
	implotColors[ImPlotCol_Selection] = ImVec4(1.00f, 0.65f, 0.00f, 1.00f);
	implotColors[ImPlotCol_Crosshairs] = ImVec4(0.23f, 0.10f, 0.64f, 0.50f);
}