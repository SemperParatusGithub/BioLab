#pragma once

#include "Core.h"

#include "Util/Buffer.h"

#include <valarray>
#include <complex>

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;


class SignalProcessing
{
public:
	static Buffer<float> Normalize(Buffer<float> buffer) {}
	static Buffer<float> Square(Buffer<float> buffer) {}

	static Buffer<float> DFT(Buffer<float> X);
	static Buffer<float> FFT(Buffer<float> X);

	static void FFT_internal(CArray& x);
	static unsigned long GetNextPowerOfTwo(unsigned long v);
};