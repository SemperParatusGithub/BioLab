#include "ECGAnalyzeWindow.h"

#include "SignalProcessing/SignalProcessing.h"
#include "SignalProcessing/DigitalFilter.h"

#include "Util/FileUtils.h"

#include <imgui.h>
#include <implot.h>



ECGAnalyzeWindow::ECGAnalyzeWindow()
{
}
ECGAnalyzeWindow::~ECGAnalyzeWindow()
{
}

void ECGAnalyzeWindow::ProcessSignal(Signal signal)
{
	Clear();
	Open();

	m_ECGData.ecgSignal = signal;

	Signal returnSignal;
	returnSignal.id = FileUtils::GetNextSignalID();
	returnSignal.color = FileUtils::GetNextColor();
	returnSignal.label += " Detection";

	for (int i = 0; i < signal.xValues.size(); i += 10)
	{
		returnSignal.xValues.push_back(signal.xValues[i]);
		returnSignal.yValues.push_back(signal.yValues[i]);
	}

	IIRFilter lowPass(
		{ 1, 0, 0, 0, 0, 0, -2, 0, 0, 0, 0, 0, 1 },
		{ 1, -2, 1 }
	);
	returnSignal = lowPass.ProcessSignal(returnSignal);

	IIRFilter highPass(
		{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, -32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, -1 }
	);
	returnSignal = highPass.ProcessSignal(returnSignal);

	IIRFilter derivative(
		{ -0.125f, -0.25f, 0.25f, 0.125f },
		{}
	);
	returnSignal = derivative.ProcessSignal(returnSignal);

	// Square
	for (auto& val : returnSignal.yValues)
		val = val * val;

	IIRFilter integrator(
		{ 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, 0.03125f, },
		{}
	);
	returnSignal = integrator.ProcessSignal(returnSignal);

	// Normalize
	float maxElem = *std::max_element(returnSignal.yValues.begin(), returnSignal.yValues.end());
	for (auto& val : returnSignal.yValues)
		val /= maxElem;

	float threshold = 0.3f;
	for (auto& val : returnSignal.yValues)
		val = val > threshold ? 1.0f : 0.0f;

	int n = 40;
	std::rotate(returnSignal.yValues.begin(), returnSignal.yValues.begin() + n, returnSignal.yValues.end());

	std::vector<float> peakBegins, peakEnds;
	bool inPeak = false;
	for (int i = 0; i < returnSignal.yValues.size(); i++)
	{
		if (!inPeak)
		{
			if (returnSignal.yValues[i] >= threshold)
			{
				inPeak = true;
				peakBegins.push_back(i * 10);
				LOG_INFO("Peak start: %d", i);
			}
		}

		if (inPeak)
		{
			if (returnSignal.yValues[i] < threshold)
			{
				inPeak = false;
				peakEnds.push_back(i * 10);
				LOG_INFO("Peak end: %d", i);
			}
		}
	}

	// R-Peaks
	for (int i = 1; i < peakEnds.size(); i++)
	{
		int startIndex = peakBegins[i];
		int endIndex = peakEnds[i];

		auto& rLocation = std::max_element(signal.yValues.begin() + startIndex, signal.yValues.begin() + endIndex);
		auto& qLocation = std::min_element(signal.yValues.begin() + startIndex, rLocation);
		auto& sLocation = std::min_element(rLocation, signal.yValues.begin() + endIndex);

		int qIndex = std::distance(signal.yValues.begin(), qLocation);
		int rIndex = std::distance(signal.yValues.begin(), rLocation);
		int sIndex = std::distance(signal.yValues.begin(), sLocation);

		m_ECGData.qx.push_back(signal.xValues[qIndex]);
		m_ECGData.qy.push_back(signal.yValues[qIndex]);

		m_ECGData.rx.push_back(signal.xValues[rIndex]);
		m_ECGData.ry.push_back(signal.yValues[rIndex]);

		m_ECGData.sx.push_back(signal.xValues[sIndex]);
		m_ECGData.sy.push_back(signal.yValues[sIndex]);
	}

	m_ECGData.avgRR = (m_ECGData.rx.back() - m_ECGData.rx.front()) / m_ECGData.rx.size();
	m_ECGData.avgBPM = 60.0f / m_ECGData.avgRR;
}

void ECGAnalyzeWindow::Render()
{
	if (!m_IsOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(m_Name.c_str(), &m_IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::BeginChild("ChildLeft", ImVec2(200.0f, -1), true);
		ImGui::Text("average RR: %.2f", m_ECGData.avgRR);
		ImGui::Text("average BPM: %.2f", m_ECGData.avgBPM);
		ImGui::EndChild();

		ImGui::SameLine();

		std::string plotName = "##" + m_Name;
		ImPlot::BeginPlot(plotName.c_str(), ImGui::GetContentRegionAvail());

		m_ECGData.ecgSignal.Plot();
		if (ImPlot::BeginLegendPopup(m_ECGData.ecgSignal.label.c_str()))
		{
			ImGui::SliderFloat("Thickness", &m_ECGData.ecgSignal.thickness, 1.0f, 8.0f);
			ImGui::SliderFloat("Alpha", &m_ECGData.ecgSignal.alpha, 0.0f, 1.0f);
			ImGui::ColorEdit3("Color", (float*)&m_ECGData.ecgSignal.color);
			ImGui::Checkbox("Markers", &m_ECGData.ecgSignal.markers);
			ImGui::Checkbox("Shaded", &m_ECGData.ecgSignal.shaded);
			ImGui::SliderInt("Stride", &m_ECGData.ecgSignal.stride, 1, 25);

			ImPlot::EndLegendPopup();
		}

		if (m_ECGData.rx.size() != 0)
		{
			ImPlot::PlotScatter("Q", m_ECGData.qx.data(), m_ECGData.qy.data(), m_ECGData.qx.size());
			ImPlot::PlotScatter("R", m_ECGData.rx.data(), m_ECGData.ry.data(), m_ECGData.rx.size());
			ImPlot::PlotScatter("S", m_ECGData.sx.data(), m_ECGData.sy.data(), m_ECGData.sx.size());
		}

		ImPlot::EndPlot();
	}
	ImGui::End();
}
void ECGAnalyzeWindow::Clear()
{
	m_ECGData.ecgSignal = Signal();
	m_ECGData.heartRate = Signal();

	m_ECGData.qx.clear();
	m_ECGData.qy.clear();
	m_ECGData.rx.clear();
	m_ECGData.ry.clear();
	m_ECGData.sx.clear();
	m_ECGData.sy.clear();
}