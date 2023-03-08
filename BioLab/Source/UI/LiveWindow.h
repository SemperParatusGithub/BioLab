#pragma once
#include "Core.h"

#include "Util/Buffer.h"

#include <string>
#include <vector>


class Application;

class LiveWindow
{
public:
	LiveWindow();
	~LiveWindow();

	void ClearBuffers();
	void AddNewSample(const Vector4f& sample);

	void Open() { m_IsOpen = true; }
	void Close() { m_IsOpen = false; }

	void Render();

private:
	bool m_IsOpen = true;

	LiveBuffer<float> m_xValues;

	LiveBuffer<float> m_Channel1;
	LiveBuffer<float> m_Channel2;
	LiveBuffer<float> m_Channel3;
};