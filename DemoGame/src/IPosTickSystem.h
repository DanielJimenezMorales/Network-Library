#pragma once

namespace ECS
{
	class EntityContainer;
}

class IPosTickSystem
{
	public:
		virtual void PosTick( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const = 0;

	protected:
		IPosTickSystem() {}
};
