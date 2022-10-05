#pragma once

#include <iostream>


class Console
{
public:
	static bool Create(std::uint32_t sizeX = 1024, std::uint32_t sizeY = 1024);
	static bool Release();
};