#include <cassert>

#include "AlgorithmUtils.h"

namespace Common
{
	uint16 AlgorithmUtils::ExponentialMovingAverage( uint16 previousValue, uint16 currentValue, uint32 alfa )
	{
		assert( alfa <= 100 );

		uint16 emaValue = ( ( ( 100 - alfa ) * previousValue ) + ( alfa * currentValue ) ) / 100;
		return emaValue;
	}

	float32 AlgorithmUtils::Lerp( float32 start, float32 end, float32 t )
	{
		return start + ( ( end - start ) * t );
	}
}
