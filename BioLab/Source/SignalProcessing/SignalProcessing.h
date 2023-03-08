#pragma once

#include "Core.h"

#include "Util/Buffer.h"


class SignalProcessing
{
public:
	static Buffer<float> Normalize(Buffer<float> buffer) {}
	static Buffer<float> Square(Buffer<float> buffer) {}
};