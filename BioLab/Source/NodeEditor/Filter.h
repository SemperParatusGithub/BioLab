#pragma once

#include "Core.h"
#include "Node.h"


class Filter : public Node
{
public:
	Filter(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Filter();

	virtual void Render() override;
	virtual float ProcessSample(float newSample) override;

private:
	std::vector<float> B = { 0.2929f, 0.5858, 0.2929f };
	std::vector<float> A = { 1.0f, 0.0f, 0.1716 };

	std::vector<float> m_InputBuffer = { 0.0f, 0.0f, 0.0f };
	std::vector<float> m_OutputBuffer = { 0.0f, 0.0f, 0.0f };
};