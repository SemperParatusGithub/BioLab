#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"
#include "SignalProcessing/Signal.h"


class InputSignal : public Node
{
public:
	InputSignal(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~InputSignal();

	virtual void Render();
	virtual Signal ProcessSignal(const Signal& signal) override;

	int GetSignalID() const { return m_SignalID; }

private:
	int m_SignalID = -1;
};


class OutputSignal : public Node
{
public:
	OutputSignal(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~OutputSignal();

	virtual void Render() override;
	virtual Signal ProcessSignal(const Signal& signal) override;

	Signal GetSignal() { return m_Signal; }

private:
	Signal m_Signal;
	char m_SignalName[128] = "OutputSigal";
};


class FourierTransform : public Node
{
public:
	FourierTransform(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~FourierTransform();

	virtual void Render() override;
	virtual Signal ProcessSignal(const Signal& signal) override;

private:
	int m_fs = 2000;
};


class RemoveDC : public Node
{
public:
	RemoveDC(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~RemoveDC();

	virtual void Render() override;
	virtual Signal ProcessSignal(const Signal& signal) override;
};


class Normalize : public Node
{
public:
	Normalize(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Normalize();

	virtual void Render() override;
	virtual Signal ProcessSignal(const Signal& signal) override;
};