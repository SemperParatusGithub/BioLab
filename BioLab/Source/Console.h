#pragma once

#include "Core.h"


class Console
{
public:
	static bool Create(const Vector2f& consoleSize = { 50.0f, 20.0f }, const Vector2f& bufferSize = { 1024.0f, 1024.0f });
	static bool Release();
};