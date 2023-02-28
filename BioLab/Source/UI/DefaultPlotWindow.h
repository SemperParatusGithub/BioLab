#pragma once
#include "Core.h"

#include <string>
#include <vector>


class Application;

class DefaultPlotWindow
{
public:
	DefaultPlotWindow();
	~DefaultPlotWindow();

	void Render();

private:
	bool m_IsOpen = true;
	std::string m_Name = "Default Plot Window";		// unused...
};