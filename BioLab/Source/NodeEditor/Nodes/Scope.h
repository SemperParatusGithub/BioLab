#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"
#include "Util/LiveBuffer.h"


class Scope : public Node
{
public:
	Scope(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Scope();

	virtual void Render() override;
	virtual float ProcessSample(float newSample) override;

	void ClearBuffer();

public:
	LiveBuffer<float> Samples;
};