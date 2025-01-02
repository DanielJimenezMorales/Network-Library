#pragma once
#include "numeric_types.h"

namespace ECS
{
	class EntityContainer;
}

class ITickSystem
{
	public:
		virtual void Tick( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const = 0;

	protected:
		ITickSystem() {}
};
