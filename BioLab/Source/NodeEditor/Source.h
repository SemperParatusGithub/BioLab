#pragma once
#include "Node.h"


class Source : public Node
{
public:
	Source(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Source();

	virtual NodeType GetNodeType() const override;
	virtual void Render() override;
};