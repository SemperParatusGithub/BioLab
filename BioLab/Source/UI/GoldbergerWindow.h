#pragma once

#include "Core.h"

#include "Util/Buffer.h"


class GoldbergerWindow
{
public:
	GoldbergerWindow();
	~GoldbergerWindow();

	void ClearBuffers();
	void AddNewSample(const Vector4f& sample);

	void Render();

	void Open() { m_IsOpen = true; }
	void Close() { m_IsOpen = false; }

private:
	bool m_IsOpen = true;

	LiveBuffer<float> m_AVR;
	LiveBuffer<float> m_AVL;
	LiveBuffer<float> m_AVF;
};