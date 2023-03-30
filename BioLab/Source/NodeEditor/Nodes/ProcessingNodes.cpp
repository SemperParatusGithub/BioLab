#include "ProcessingNodes.h"

#include <string>


Gain::Gain(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Gain;
}
Gain::~Gain()
{
}

void Gain::Render()
{
	float regionWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetNextItemWidth(regionWidth);
	ImGui::DragFloat("##Gain", &m_Gain, 0.05f);
}
float Gain::ProcessSample(float newSample)
{
	return newSample * m_Gain;
}

Signal Gain::ProcessSignal(const Signal& signal)
{
	Signal out = signal;

	for (float& y : out.yValues)
		y *= m_Gain;

	return out;
}



Offset::Offset(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Offset;
}
Offset::~Offset()
{
}

void Offset::Render()
{
	float regionWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetNextItemWidth(regionWidth);
	ImGui::DragFloat("##Offset", &m_Offset, 1.0f);
}
float Offset::ProcessSample(float newSample)
{
	return newSample + m_Offset;
}

Signal Offset::ProcessSignal(const Signal& signal)
{
	Signal out = signal;

	for (float& y : out.yValues)
		y += m_Offset;

	return out;
}



Absolute::Absolute(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Absolute;
}
Absolute::~Absolute()
{
}

void Absolute::Render()
{
	ImGui::Checkbox("Invert", &m_Invert);
}
float Absolute::ProcessSample(float newSample)
{
	return std::abs(newSample) * (m_Invert ? -1.0f : 1.0f);
}

Signal Absolute::ProcessSignal(const Signal& signal)
{
	Signal out = signal;

	for (float& y : out.yValues)
		y = abs(y) * (m_Invert ? -1.0f : 1.0f);

	return out;
}

Square::Square(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Square;
}
Square::~Square()
{
}

void Square::Render()
{
}
float Square::ProcessSample(float newSample)
{
	return newSample * newSample;
}

Signal Square::ProcessSignal(const Signal& signal)
{
	Signal out = signal;

	for (float& y : out.yValues)
		y = y * y;

	return out;
}



Filter::Filter(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Filter;

	m_Filter = IIRFilter({ 0.5f, -0.5f, 0.5f }, { 1.0f, 2.0f, 3.0f });
}
Filter::~Filter()
{
}

void Filter::Render()
{
	float regionWidth = ImGui::GetContentRegionAvail().x;

	auto [B, A] = m_Filter.GetCoefficients();
	
	for (int i = 0; i < B.size(); i++)
	{
		std::string label;
		float value;
	
		label = "B" + std::to_string(i);
		value = B[i];
	
		ImGui::SetNextItemWidth(regionWidth / 2.3f - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::DragFloat(label.c_str(), &value, 0.1f))
			B[i] = value;
	
		ImGui::SameLine();
	
		label = "A" + std::to_string(i);
		value = A[i];
	
		ImGui::SetNextItemWidth(regionWidth / 2.5f - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::DragFloat(label.c_str(), &value, 0.1f))
			A[i] = value;
	}

	// Update filter
	m_Filter = IIRFilter(B, A);
	
	if (ImGui::Button("Clear buffers"))
		m_Filter.ClearBuffers();

	ImGui::SameLine();

	if (ImGui::Button("+"))
		m_Filter.AddNewCoeffcients();

	ImGui::SameLine();
	if (ImGui::Button("-"))
		m_Filter.RemoveLastCoeffcients();
}
float Filter::ProcessSample(float newSample)
{
	return m_Filter.ProcessSample(newSample);
}

Signal Filter::ProcessSignal(const Signal& signal)
{
	return m_Filter.ProcessSignal(signal);
}

Average::Average(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Average;
}
Average::~Average()
{
}

void Average::Render()
{
	if (ImGui::SliderInt("Window", &m_WindowSize, 1, 100))
		m_Buffer.resize(m_WindowSize);

	if (ImGui::Button("Clear Buffer"))
		std::fill(m_Buffer.begin(), m_Buffer.end(), 0);
}
float Average::ProcessSample(float newSample)
{
	m_Buffer.push_back(newSample / m_WindowSize);
	m_Buffer.erase(m_Buffer.begin());

	float result = 0.0f;
	for (int i = 0; i < m_WindowSize; i++)
		result += m_Buffer[i];

	return result;
}
Signal Average::ProcessSignal(const Signal& signal)
{
	Signal outputSignal = signal;

	auto inputValues = signal.yValues;
	std::vector<float> outputValues(inputValues.size());


	for (int i = 0; i < inputValues.size(); i++)
	{
		float result = 0.0f;

		// Non recursive filter factors
		for (int j = 0; j < m_WindowSize; j++)
		{
			if (i - j < 0)
				break;
			result += 1.0f / m_WindowSize * inputValues[i - j];
		}
		outputValues[i] = result;
	}

	outputSignal.yValues = outputValues;
	return outputSignal;
}