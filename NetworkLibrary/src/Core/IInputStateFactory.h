#pragma once

namespace NetLib
{
	class IInputState;

	class IInputStateFactory
	{
	public:
		virtual IInputState* Create() = 0;
		virtual void Destroy(IInputState* inputToDestroy) = 0;
	};
}
