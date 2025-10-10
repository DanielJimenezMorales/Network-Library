#pragma once

namespace Engine
{
	class Game;

	class IWorldInitializer
	{
		public:
			IWorldInitializer() {};
			virtual ~IWorldInitializer() {};

			virtual void SetUpWorld( Game& game ) = 0;
	};
}
