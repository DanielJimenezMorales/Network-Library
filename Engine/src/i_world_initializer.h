#pragma once

namespace Engine
{
	namespace ECS
	{
		class World;
	}

	class IWorldInitializer
	{
		public:
			IWorldInitializer() {};
			virtual ~IWorldInitializer() {};

			virtual void SetUpWorld( ECS::World& world ) = 0;
	};
}
