#include "DigitalFilter.h"


IIRFilter::IIRFilter()
{
}

IIRFilter::IIRFilter(const std::vector<float>& B, const std::vector<float>& A)
{
	m_B = B;
	m_A = A;

	for (int i = 0; i < B.size(); i++)
	{
		m_InputBuffer.push_back(0.0f);
		m_OutputBuffer.push_back(0.0f);
	}
}
IIRFilter::~IIRFilter()
{
}

float IIRFilter::ProcessSample(float sample)
{
    m_InputBuffer.push_back(sample);
    m_InputBuffer.erase(m_InputBuffer.begin(), m_InputBuffer.begin() + 1);

    float output = 0.0f;

    std::size_t iMaxIdx = m_InputBuffer.size() - 1;
    for (int j = 0; j < m_B.size(); j++)
        output += m_B[j] * m_InputBuffer[iMaxIdx - j];

    std::size_t oMaxIdx = m_OutputBuffer.size() - 1;
    for (int j = 1; j < m_A.size(); j++)
        output -= m_A[j] * m_OutputBuffer[oMaxIdx - j + 1];

    m_OutputBuffer.push_back(output);
    m_OutputBuffer.erase(m_OutputBuffer.begin(), m_OutputBuffer.begin() + 1);

    return output;
}
Signal IIRFilter::ProcessSignal(Signal signal)
{
	Signal outputSignal = signal;

	auto inputValues = signal.xValues;
	std::vector<float> outputValues(inputValues.size());


	for (int i = 0; i < inputValues.size(); i++)
	{
		float result = 0.0f;

		// Non recursive filter factors
		for (int j = 0; j < m_B.size(); j++)
		{
			if (i - j < 0)
				break;
			result += m_B[j] * inputValues[i - j];
		}
		// recursive filter factors
		for (int j = 0; j < m_A.size(); j++)
		{
			if (i - j < 0)
				break;
			result -= m_A[j] * outputValues[i - j];
		}
		outputValues[i] = result;
	}

	outputSignal.yValues = outputValues;
	return outputSignal;
}

void IIRFilter::ClearBuffers()
{
	std::fill(m_InputBuffer.begin(), m_InputBuffer.end(), 0);
	std::fill(m_OutputBuffer.begin(), m_OutputBuffer.end(), 0);
}