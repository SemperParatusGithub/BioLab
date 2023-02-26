#pragma once
#include "Core.h"

#include "Script.h"



class ScriptSerializer
{
public:
	void SerializeScript(Script& script, const std::string& filepath);
	Script DeserializeScript(const std::string& filepath);

	Script CreateNewLiveScript();
	Script CreateNewPostProcessScript();

private:
	void SerializeNode(Node* node, YAML::Emitter& out);
	void SerializeLink(Link link, YAML::Emitter& out);
};