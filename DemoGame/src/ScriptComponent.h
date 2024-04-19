#pragma once
#include "ScriptableSystem.h"

struct ScriptComponent
{
public:
	template <typename T>
	void Bind()
	{
		CreateScript = []() {return static_cast<ScriptableSystem*>(new T()); };
		DestroyScript = [](ScriptComponent* scriptComponent) {delete scriptComponent->behaviour; scriptComponent->behaviour = nullptr; };
	}

	bool IsCreated() const { return behaviour != nullptr; };

	ScriptableSystem* (*CreateScript)();
	void (*DestroyScript)(ScriptComponent*);
	ScriptableSystem* behaviour;
};
