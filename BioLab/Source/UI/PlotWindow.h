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

	void Render();
	void Clear();

	std::vector<int> GetSignalIDs() { return m_SignalIDs; }
	std::vector<int> GetScopeIDs() { return m_ScopeIDs; }

	void SetName(const std::string& name) { m_Name = name; }
	std::string GetName() { return m_Name; }

private:
	bool m_IsOpen = true;

	std::string m_Name = "Unnamed Plot Window";
	std::vector<int> m_SignalIDs;
	std::vector<int> m_ScopeIDs;
};