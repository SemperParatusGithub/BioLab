#pragma once
#include "Node.h"


class Scope : public Node
{
public:
	Scope(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Scope();

	virtual NodeType GetNodeType() const override;
	virtual void Render() override;
};