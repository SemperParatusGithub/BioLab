#pragma once

#include "Core.h"

#include <vector>
#include <string>
#include <fstream>

#include <implot.h>



struct Signal
{
	int id = 0;
	std::string label = "Unnamed";
	ImVec4 color;
	std::vector<float> xValues, yValues;
};

class FileUtils
{
private:
	static int GetColorMapIndex()
	{
		static int index = 0;

		index++;
		if (index >= 16)
			index = 0;

		return index;
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
		Signal sig;
		sig.id = GetNextSignalID();
		sig.label = "Signal" + std::to_string(sig.id);
		sig.color = ImPlot::GetColormapColor(GetColorMapIndex());

		LOG_INFO("Loading CSV file: %s", filepath.c_str());

		if (!filepath.empty())
		{
			std::ifstream is;
			is.open(filepath);
			std::string line;
			while (std::getline(is, line))
			{
				auto comma = line.find(',');
				float x = std::stof(line.substr(0, comma));
				float y = std::stof(line.substr(comma + 1, line.size() - comma + 1));
				sig.xValues.push_back(x);
				sig.yValues.push_back(y);

				LOG_INFO("x: %.2f, y: %.2f", x, y);
			}
		}

		return sig;
	}
	//static void WriteCSV(const std::string& filepath);
};