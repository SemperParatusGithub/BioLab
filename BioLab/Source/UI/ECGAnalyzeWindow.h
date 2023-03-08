#pragma once
#include "Core.h"

#include "Util/Buffer.h"
#include "SignalProcessing/Signal.h"


struct ECGData
{
	Signal ecgSignal;

	Buffer<float> qx, qy;
	Buffer<float> rx, ry;
	Buffer<float> sx, sy;

	float avgQR;
	float avgRS;
	float avgRR;

	float avgBPM;

	Signal heartRate;
};


class ECGAnalyzeWindow
{
public:
	ECGAnalyzeWindow();
	~ECGAnalyzeWindow();

	void ProcessSignal(Signal signal);

	void Render();
	void Clear();

	void Close() { m_IsOpen = false; }
	bool IsOpen() const { return m_IsOpen; }

private:
	bool m_IsOpen = true;
	std::string m_Name = "ECG Analyzer";

	ECGData m_ECGData;
};