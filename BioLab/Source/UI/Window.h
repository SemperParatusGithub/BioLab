#pragma once
#include "../Core.h"


struct GLFWwindow;


class Window
{
public:
	Window(const std::string& title, uint32_t width, uint32_t height);
	~Window();

	bool isOpen() const;
	void Close();

	void Maximize();

	void BeginFrame();
	void EndFrame();

	void Update();
	void Clear(const Vector4f& clearColor = { 0.7f, 0.7f, 0.7f, 0.7f });

	const GLFWwindow* GetWindow() const;

	Vector2f GetPosition() const;
	Vector2f GetSize() const;

	void SetPosition(const Vector2f& pos);
	void SetSize(const Vector2f& size);

	void SetMinimumSize(const Vector2f& size);
	void SetMaximumSize(const Vector2f& size);

private:
	GLFWwindow* m_Window;
};