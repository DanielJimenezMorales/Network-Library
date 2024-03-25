#include <cassert>

#include "AlgorithmUtils.h"

namespace NetLib
{
    uint16_t AlgorithmUtils::ExponentialMovingAverage(uint16_t previousValue, uint16_t currentValue, unsigned int alfa)
    {
        assert(alfa <= 100);

        uint16_t emaValue = (((100 - alfa) * previousValue) + (alfa * currentValue)) / 100;
        return emaValue;
    }
}
