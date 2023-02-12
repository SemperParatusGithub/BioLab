#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"


class Source : public Node
{
public:
	Source(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Source();

	virtual void Render() override;
	virtual float ProcessSample(float newSample) override;
};