#pragma once
#include "TimeClock.h"

#include "communication/message_factory.h"

namespace NetLib
{
	class Initializer
	{
	public:
		void static Initialize()
		{
			TimeClock::CreateInstance();
			MessageFactory::CreateInstance(3);
		}

		void static Finalize()
		{
			TimeClock::DeleteInstance();
			MessageFactory::DeleteInstance();
		}
	};
}
