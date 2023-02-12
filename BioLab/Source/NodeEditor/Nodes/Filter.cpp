#include "Filter.h"

#include <string>


Filter::Filter(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size)
{
	this->name = nodeName;
	this->id = nodeID;
	this->position = position;
	this->size = size;

	this->type = Node::Type::Filter;
}
Filter::~Filter()
{
}

void Filter::Render()
{
	float regionWidth = ImGui::GetContentRegionAvail().x;
	
	for (int i = 0; i < this->B.size(); i++)
	{
		std::string label;
		float value;

		label = "B" + std::to_string(i);
		value = this->B[i];

		ImGui::SetNextItemWidth(regionWidth / 2.3f - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::DragFloat(label.c_str(), &value, 0.1f))
			this->B[i] = value;	
		
		ImGui::SameLine();

		label = "A" + std::to_string(i);
		value = this->A[i];

		ImGui::SetNextItemWidth(regionWidth / 2.5f - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::DragFloat(label.c_str(), &value, 0.1f))
			this->A[i] = value;
	}
	
	if (ImGui::Button("Clear buffers"))
	{
		std::fill(m_InputBuffer.begin(), m_InputBuffer.end(), 0);
		std::fill(m_OutputBuffer.begin(), m_OutputBuffer.end(), 0);
	}
}
float Filter::ProcessSample(float newSample)
{
	m_InputBuffer.push_back(newSample);
	m_InputBuffer.erase(m_InputBuffer.begin(), m_InputBuffer.begin() + 1);

	float output = 0.0f;
	std::size_t iMaxIdx = m_InputBuffer.size() - 1;
	output += B[0] * m_InputBuffer[iMaxIdx];
	output += B[1] * m_InputBuffer[iMaxIdx - 1];
	output += B[2] * m_InputBuffer[iMaxIdx - 2];

	std::size_t oMaxIdx = m_OutputBuffer.size() - 1;
	output -= A[1] * m_OutputBuffer[oMaxIdx];
	output -= A[1] * m_OutputBuffer[oMaxIdx - 1];

	m_OutputBuffer.push_back(output);
	m_OutputBuffer.erase(m_OutputBuffer.begin(), m_OutputBuffer.begin() + 1);

	return output;
}