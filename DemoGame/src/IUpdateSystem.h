#pragma once
#include "numeric_types.h"

namespace ECS
{
	class EntityContainer;
}

class IUpdateSystem
{
	public:
		virtual void Update( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const = 0;

	protected:
		IUpdateSystem() {}
};
