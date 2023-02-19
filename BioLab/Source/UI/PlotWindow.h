#pragma once
#include "Core.h"

#include <string>
#include <vector>


class Application;

class PlotWindow
{
public:
	PlotWindow();
	~PlotWindow();

	void Begin();
	void End();

	std::vector<int> GetSignalIDs() { return m_SignalIDs; }
	std::string GetName() { return m_Name; }

private:
	std::string m_Name = "Test Plot Window";
	std::vector<int> m_SignalIDs;
	bool m_IsOpen = true;
	bool m_AutoLiveScroll = false;

	friend class Application;
};