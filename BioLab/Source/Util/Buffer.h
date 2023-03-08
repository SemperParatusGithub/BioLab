#pragma once
#include "Core.h"

#include <vector>	


const std::size_t maxSize = 1000;
const std::size_t clearSize = 250;

template<typename T>
using Buffer = std::vector<T>;

template<typename T>
class LiveBuffer
{
public:
	LiveBuffer()
	{
	}
	~LiveBuffer()
	{
	}

	void PushBack(const T& value)
	{
		data.push_back(value);

		if (data.size() >= maxSize)
			data.erase(data.begin(), data.begin() + clearSize);
	}

	std::size_t Size() const
	{
		return data.size();
	}

	void Clear()
	{
		data.clear();
	}

	T* Data()
	{
		return data.data();
	}
	const T* Data() const
	{
		return data.data();
	}

	T& Back()
	{
		return data.back();
	}

public:
	std::vector<T> data;
};