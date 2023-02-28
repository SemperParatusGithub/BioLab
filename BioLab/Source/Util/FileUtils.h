#pragma once

#include "Core.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include <implot.h>

#include "Signal.h"


class FileUtils
{
public:
	static ImVec4 GetNextColor()
	{
		static int index = 0;

		index++;
		if (index >= 16)
			index = 0;

		return ImPlot::GetColormapColor(index);
	}
	static int GetNextSignalID()
	{
		static int id = 0;

		id++;
		return id;
	}

public:
	static Signal LoadCSV(const std::string& filepath)
	{
		if (filepath.empty())
		{
			LOG_ERROR("Failed to load CSV file: %s", filepath.c_str());
			return Signal();
		}

		Signal signal;
		signal.id = GetNextSignalID();
		signal.label = filepath.substr(filepath.find_last_of("\\") + 1);
		signal.color = GetNextColor();

		LOG_INFO("Loading CSV file: %s", filepath.c_str());		

		std::ifstream is;
		is.open(filepath);
		std::string line;
		while (std::getline(is, line))
		{
			auto comma = line.find(',');
			float x = std::stof(line.substr(0, comma));
			float y = std::stof(line.substr(comma + 1, line.size() - comma + 1));
			signal.xValues.push_back(x);
			signal.yValues.push_back(y);

			// LOG_INFO("x: %.2f, y: %.2f", x, y);
		}

		return signal;
	}

	static void SaveCSV(const Signal& signal, const std::string& filepath)
	{
		LOG_INFO("Writing CSV file: %s", filepath.c_str());

		std::string newFilepath = filepath;
		std::replace(newFilepath.begin(), newFilepath.end(), 'x', '_');

		LOG_INFO("Writing CSV file: %s", newFilepath.c_str());

		if (!newFilepath.empty())
		{
			std::ofstream out;
			out.open(newFilepath);

			for (int i = 0; i < signal.xValues.size(); i++)
				out << signal.xValues[i] << "," << signal.yValues[i] << std::endl;

			out.flush();
		}
	}
};