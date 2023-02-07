#include "UICore.h"
#include "Core.h"

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