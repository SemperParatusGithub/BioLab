#include "UICore.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <imgui_internal.h>

#include <implot.h>

#include <GLFW/glfw3.h>


static inline ImVec2 operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static inline ImVec2 operator/(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const float rhs) { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const float rhs) { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static inline ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
static inline ImVec4 operator-(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }


void UICore::Initialize(GLFWwindow* window, const char* glslVersion)
{
	// ImGui Setup
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

	// Setup Dear ImGui style
	ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);

	SetStyle();

	SetLightColorTheme();
	// SetDarkColorTheme();
}
void UICore::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void UICore::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void UICore::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void UICore::SetStyle()
{	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ImGui styling
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ImGuiStyle* imguiStyle = &ImGui::GetStyle();

	imguiStyle->WindowMinSize = ImVec2(10, 10);
	imguiStyle->WindowPadding = ImVec2(10, 10);
	imguiStyle->WindowRounding = 5.0f;
	imguiStyle->FramePadding = ImVec2(5, 5);
	imguiStyle->FrameRounding = 5.0f;
	imguiStyle->ItemSpacing = ImVec2(12, 8);
	imguiStyle->ItemInnerSpacing = ImVec2(8, 6);
	imguiStyle->IndentSpacing = 25.0f;
	imguiStyle->ScrollbarSize = 15.0f;
	imguiStyle->ScrollbarRounding = 5.0f;
	imguiStyle->GrabRounding = 5.0f;
	imguiStyle->TabRounding = 5.0f;
	imguiStyle->ChildRounding = 5.0f;
	imguiStyle->PopupRounding = 5.0f;
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
	imguiColors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	imguiColors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
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

void UICore::SetDarkColorTheme()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ImGui colors
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ImGuiStyle* imguiStyle = &ImGui::GetStyle();
	ImVec4* imguiColors = imguiStyle->Colors;

	imguiColors[ImGuiCol_Text] = ImVec4(0.96f, 0.96f, 0.99f, 1.00f);
	imguiColors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	imguiColors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	imguiColors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
	imguiColors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
	imguiColors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
	imguiColors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
	imguiColors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	imguiColors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
	imguiColors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	imguiColors[ImGuiCol_FrameBgActive] = ImVec4(0.27f, 0.27f, 0.29f, 1.00f);
	imguiColors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	imguiColors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	imguiColors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	imguiColors[ImGuiCol_MenuBarBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	imguiColors[ImGuiCol_ScrollbarBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	imguiColors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.32f, 1.00f);
	imguiColors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.42f, 1.00f);
	imguiColors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.53f, 1.00f);
	imguiColors[ImGuiCol_CheckMark] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
	imguiColors[ImGuiCol_SliderGrab] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
	imguiColors[ImGuiCol_SliderGrabActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
	imguiColors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	imguiColors[ImGuiCol_ButtonHovered] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
	imguiColors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
	imguiColors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	imguiColors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
	imguiColors[ImGuiCol_HeaderActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
	imguiColors[ImGuiCol_Separator] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
	imguiColors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.47f, 0.39f);
	imguiColors[ImGuiCol_SeparatorActive] = ImVec4(0.44f, 0.44f, 0.47f, 0.59f);
	imguiColors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
	imguiColors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
	imguiColors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
	imguiColors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	imguiColors[ImGuiCol_TabHovered] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
	imguiColors[ImGuiCol_TabActive] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
	imguiColors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.20f, 0.22f, 0.39f);
	imguiColors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.44f, 0.44f, 0.47f, 0.39f);
	imguiColors[ImGuiCol_DockingPreview] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
	imguiColors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	imguiColors[ImGuiCol_DragDropTarget] = ImVec4(0.91f, 0.62f, 0.00f, 1.00f);
	imguiColors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	imguiColors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	imguiColors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	imguiColors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ImPlot colors
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ImPlotStyle* implotStyle = &ImPlot::GetStyle();
	ImVec4* implotColors = implotStyle->Colors;

	implotColors[ImPlotCol_Line] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_Fill] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_MarkerFill] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_ErrorBar] = IMPLOT_AUTO_COL;
	implotColors[ImPlotCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	implotColors[ImPlotCol_PlotBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	implotColors[ImPlotCol_PlotBorder] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	implotColors[ImPlotCol_LegendBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	implotColors[ImPlotCol_LegendBorder] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	implotColors[ImPlotCol_LegendText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	implotColors[ImPlotCol_TitleText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	implotColors[ImPlotCol_InlayText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	implotColors[ImPlotCol_AxisText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	implotColors[ImPlotCol_AxisGrid] = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
	implotColors[ImPlotCol_AxisTick] = IMPLOT_AUTO_COL; // TODO
	implotColors[ImPlotCol_AxisBg] = IMPLOT_AUTO_COL; // TODO
	implotColors[ImPlotCol_AxisBgHovered] = IMPLOT_AUTO_COL; // TODO
	implotColors[ImPlotCol_AxisBgActive] = IMPLOT_AUTO_COL; // TODO
	implotColors[ImPlotCol_Selection] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	implotColors[ImPlotCol_Crosshairs] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
}