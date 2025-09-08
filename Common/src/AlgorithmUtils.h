#pragma once
#include "numeric_types.h"

namespace Common
{
	class AlgorithmUtils
	{
	public:
		static uint16 ExponentialMovingAverage(uint16 previousValue, uint16 currentValue, uint32 alfa);
		static float32 Lerp( float32 start, float32 end, float32 t );
	};
}
