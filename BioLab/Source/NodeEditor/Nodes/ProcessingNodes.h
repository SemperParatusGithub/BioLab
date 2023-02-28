#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"
#include "Util/Signal.h"


class Gain : public Node
{
public:
	Gain(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Gain();

	virtual void Render() override;
	virtual float ProcessSample(float newSample) override;
	virtual Signal ProcessSignal(const Signal& signal) override;

private:
	float m_Gain = 1.0f;
};


class Offset : public Node
{
public:
	Offset(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Offset();

	virtual void Render() override;
	virtual float ProcessSample(float newSample) override;
	virtual Signal ProcessSignal(const Signal& signal) override;

private:
	float m_Offset = 1.0f;
};


class Absolute : public Node
{
public:
	Absolute(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Absolute();

	virtual void Render() override;
	virtual float ProcessSample(float newSample) override;
	virtual Signal ProcessSignal(const Signal& signal) override;

private:
	float m_Offset = 1.0f;
	bool m_Invert = false;
};