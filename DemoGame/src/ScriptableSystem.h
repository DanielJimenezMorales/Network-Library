#pragma once
#include "GameEntity.h"

class ScriptableSystem
{
public:
	virtual ~ScriptableSystem() {};

protected:
	virtual void Create() {};
	virtual void Update(float elapsedTime) {};
	virtual void Tick(float tickElapsedTime) {};
	virtual void Destroy() {};

	template <typename T>
	T& GetComponent();

private:
	GameEntity entity;
	friend class ScriptSystem;
};

template<typename T>
inline T& ScriptableSystem::GetComponent()
{
	return entity.GetComponent<T>();
};
