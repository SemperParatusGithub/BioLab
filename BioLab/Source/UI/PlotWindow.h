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

	void EnableDragDrop() { m_DragDropTarget = true; }
	void DisableDragDrop() { m_DragDropTarget = false; }

	void AddSignalID(SignalID id) { m_SignalIDs.push_back(id); }

	std::vector<int> GetSignalIDs() { return m_SignalIDs; }
	std::vector<int> GetScopeIDs() { return m_ScopeIDs; }

	void SetName(const std::string& name) { m_Name = name; }
	std::string GetName() { return m_Name; }

	bool IsOpen() const { return m_IsOpen; }

private:
	bool m_IsOpen = true;
	bool m_DragDropTarget = true;

	std::string m_Name = "Unnamed Plot Window";
	std::vector<int> m_SignalIDs;
	std::vector<int> m_ScopeIDs;
};