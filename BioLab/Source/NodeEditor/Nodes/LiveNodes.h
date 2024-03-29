#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"
#include "Util/Buffer.h"


class Source : public Node
{
public:
	Source(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Source();

	virtual void Render() override;
	virtual float ProcessSample(float newSample) override;
};


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