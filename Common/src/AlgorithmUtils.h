#pragma once
#include "NumericTypes.h"

namespace Common
{
	class AlgorithmUtils
	{
	public:
		static uint16 ExponentialMovingAverage(uint16 previousValue, uint16 currentValue, uint32 alfa);
	};
}
