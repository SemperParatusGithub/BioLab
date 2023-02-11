#include "Core.h"


namespace OpenGL
{
	void Initialize(const std::string& title, u32 width, u32 height);
	void Shutdown();

	bool WindowOpen();

	void BeginFrame();
	void EndFrame();

	void PollEvents();

	u64 CreateTexture(const void* data, int width, int height);
}