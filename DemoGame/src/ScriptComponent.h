#pragma once

class ScriptableSystem;

struct ScriptComponent
{
public:
	template <typename T>
	void Bind()
	{
		behaviour = static_cast<ScriptableSystem*>(new T());
		isCreated = false;
	}

	void Unbind()
	{
		delete behaviour;
		behaviour = nullptr;
	}

	ScriptableSystem* behaviour;
	bool isCreated;
};
