#include "SignalProcessing.h"

#include <complex>


Buffer<float> SignalProcessing::DFT(Buffer<float> X)
{
	float PI = static_cast<float>(3.14159265358979323846);

	int N = X.size();
	int K = N;

	std::complex<float> intSum;
	Buffer<float> output;
	output.reserve(K);

	for (int k = 0; k < K; k++)
	{
		intSum = std::complex<float>(0, 0);

		for (int n = 0; n < N; n++)
		{
			float realPart = std::cos((2 * PI / N) * k * n);
			float imagPart = std::sin((2 * PI / N) * k * n);

			std::complex<float> w(realPart, -imagPart);
			intSum += std::complex(X[n], 0.0f) * w;
		}

		output.push_back(std::abs(intSum));
	}

	return output;	
}