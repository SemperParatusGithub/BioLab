#include "UICore.h"
#include "Core.h"

#include <stb_image.h>

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

u64 UICore::LoadTexture(const std::string& filepath)
{
	LOG_INFO("Loading texture: %s", filepath);

	int width = 0, height = 0, channels = 0;

	if (stbi_uc* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4))
	{
		auto textureHandle = Backend::CreateTexture(data, width, height);

		return textureHandle;
	}

	LOG_ERROR("Failed to load texture: %s", filepath);
	return 0;
}
