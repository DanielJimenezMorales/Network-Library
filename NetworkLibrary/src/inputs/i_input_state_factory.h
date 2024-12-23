#pragma once

namespace NetLib
{
	class IInputState;

	class IInputStateFactory
	{
		public:
			IInputStateFactory() {}
			virtual ~IInputStateFactory() {}

			virtual IInputState* Create() = 0;
			virtual void Destroy( IInputState* inputToDestroy ) = 0;
	};
}
