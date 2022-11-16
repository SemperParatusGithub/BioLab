#pragma once
#include "Node.h"


class Comment : public Node
{
public:
	Comment(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Comment();

	virtual NodeType GetNodeType() const override;
	virtual void Render() override;
};