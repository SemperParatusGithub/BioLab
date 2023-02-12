#pragma once
#include "Core.h"

#include "NodeEditor/Node.h"


class Comment : public Node
{
public:
	Comment(ax::NodeEditor::NodeId nodeID, const std::string& nodeName, const Vector2f& position, const Vector2f& size);
	~Comment();

	virtual void Render() override;
};