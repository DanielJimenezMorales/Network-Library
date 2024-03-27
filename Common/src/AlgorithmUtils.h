#pragma once
#include <cstdint>

namespace Common
{
	class AlgorithmUtils
	{
	public:
		static uint16_t ExponentialMovingAverage(uint16_t previousValue, uint16_t currentValue, unsigned int alfa);
	};
}
