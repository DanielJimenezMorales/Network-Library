#include <cassert>

#include "AlgorithmUtils.h"

namespace Common
{
    uint16 AlgorithmUtils::ExponentialMovingAverage(uint16 previousValue, uint16 currentValue, uint32 alfa)
    {
        assert(alfa <= 100);

        uint16 emaValue = (((100 - alfa) * previousValue) + (alfa * currentValue)) / 100;
        return emaValue;
    }
}
