#include "Core.h"


namespace DirectX 
{
	void Initialize(const std::string& title, u32 width, u32 height);
	void Shutdown();

	bool WindowOpen();

	void BeginFrame();
	void EndFrame();

	void PollEvents();
}