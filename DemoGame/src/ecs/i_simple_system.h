#pragma once

class GameEntity;

namespace ECS
{
	class ISimpleSystem
	{
		public:
			ISimpleSystem() {}
			virtual ~ISimpleSystem() {}

			virtual void Execute( GameEntity& entity ) = 0;
	};
}
