#include "SignalProcessing.h"


const double PI = 3.141592653589793238460;


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

Buffer<float> SignalProcessing::FFT(Buffer<float> X)
{
	int input_size = X.size();
	std::vector<float> returnValues = X;
	returnValues.resize(GetNextPowerOfTwo(input_size));
	std::fill(returnValues.begin() + input_size, returnValues.end(), 0);

	std::valarray<Complex> valar(returnValues.size());
	for (int i = 0; i < valar.size(); i++)
		valar[i] = returnValues[i];

	FFT_internal(valar);

	for (int i = 0; i < valar.size(); i++)
		returnValues[i] = std::abs(valar[i]);

	return returnValues;
}

void SignalProcessing::FFT_internal(CArray& x)
{
	const size_t N = x.size();
	if (N <= 1) return;

	// divide
	CArray even = x[std::slice(0, N / 2, 2)];
	CArray  odd = x[std::slice(1, N / 2, 2)];

	// conquer
	FFT_internal(even);
	FFT_internal(odd);

	// combine
	for (size_t k = 0; k < N / 2; ++k)
	{
		Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
		x[k] = even[k] + t;
		x[k + N / 2] = even[k] - t;
	}
}

unsigned long SignalProcessing::GetNextPowerOfTwo(unsigned long v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}