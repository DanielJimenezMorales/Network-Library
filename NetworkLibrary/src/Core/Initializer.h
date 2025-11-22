#pragma once
#include "core/time_clock.h"

namespace NetLib
{
	class Initializer
	{
		public:
			void static Initialize() { TimeClock::CreateInstance(); }

			void static Finalize() { TimeClock::DeleteInstance(); }
	};
}
