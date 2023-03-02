#pragma once
#include "Core.h"
#include "Util/Signal.h"

#include <tuple>


class IIRFilter
{
public:
	IIRFilter();
	IIRFilter(const std::vector<float>& B, const std::vector<float>& A);
	~IIRFilter();

	float ProcessSample(float sample);
	Signal ProcessSignal(Signal signal);

	void ClearBuffers();

	std::pair<std::vector<float>, std::vector<float>> GetCoefficients() { return { m_B, m_A }; }

	void AddNewCoeffcients();
	void RemoveLastCoeffcients();

private:
	std::vector<float> m_B;
	std::vector<float> m_A;

	std::vector<float> m_InputBuffer;
	std::vector<float> m_OutputBuffer;
};