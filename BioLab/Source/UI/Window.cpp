#include "Window.h"

#include "UICore.h"

#include <GLFW/glfw3.h>


static uint32_t s_WindowCount = 0;

Window::Window(const std::string& title, uint32_t width, uint32_t height)
{
	if (s_WindowCount != 0)
		__debugbreak();

	if (!glfwInit())
		__debugbreak();

	// GL 3.0 and GLSL 130
	const char* glslVersion = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	m_Window = glfwCreateWindow(
		static_cast<int>(width), 
		static_cast<int>(height), 
		title.c_str(), 
		nullptr, 
		nullptr);

	if (!m_Window)
		__debugbreak();

	glfwMakeContextCurrent(m_Window);

	UICore::Initialize(m_Window, glslVersion);

	s_WindowCount++;
}
Window::~Window()
{
	UICore::Shutdown();

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

bool Window::isOpen() const
{
	return !glfwWindowShouldClose(m_Window);
}
void Window::Close()
{
	glfwSetWindowShouldClose(m_Window, true);
}

void Window::Maximize()
{
	glfwMaximizeWindow(m_Window);
}

void Window::BeginFrame()
{
	UICore::BeginFrame();
}
void Window::EndFrame()
{
	UICore::EndFrame();
}

void Window::Update()
{
	glfwSwapBuffers(m_Window);
	glfwPollEvents();
}
void Window::Clear(const Vector4f& clearColor)
{
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT);
}

const GLFWwindow* Window::GetWindow() const
{
	return m_Window;
}

Vector2f Window::GetPosition() const
{
	int xPos, yPos;
	glfwGetWindowPos(m_Window, &xPos, &yPos);
	return Vector2f { 
		static_cast<float>(xPos),
		static_cast<float>(yPos) };
}
Vector2f Window::GetSize() const
{
	int xSize, ySize;
	glfwGetWindowSize(m_Window, &xSize, &ySize);

	return Vector2f { 
		static_cast<float>(xSize), 
		static_cast<float>(ySize) };
}

void Window::SetPosition(const Vector2f& pos)
{
	glfwSetWindowPos(m_Window, 
		static_cast<int>(pos.x), 
		static_cast<int>(pos.y));
}
void Window::SetSize(const Vector2f& size)
{
	glfwSetWindowSize(m_Window, 
		static_cast<int>(size.x), 
		static_cast<int>(size.y));
}

void Window::SetMinimumSize(const Vector2f& size)
{
	glfwSetWindowSizeLimits(m_Window,
		static_cast<int>(size.x),
		static_cast<int>(size.y),
		GLFW_DONT_CARE, 
		GLFW_DONT_CARE);
}
void Window::SetMaximumSize(const Vector2f& size)
{
	glfwSetWindowSizeLimits(m_Window,
		GLFW_DONT_CARE,
		GLFW_DONT_CARE,
		static_cast<int>(size.x),
		static_cast<int>(size.y));
}