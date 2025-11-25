#pragma once
#include "numeric_types.h"

#include "metrics/metric_types.h"

#include <string>

namespace NetLib
{
	namespace Metrics
	{
		class IMetric
		{
			public:
				virtual MetricType GetType() const = 0;
				virtual uint32 GetValue( ValueType value_type ) const = 0;
				virtual void SetUpdateRate( float32 update_rate ) = 0;
				virtual void Update( float32 elapsed_time ) = 0;
				virtual void AddValueSample( uint32 value, const std::string& sample_type = "NONE" ) = 0;
				virtual void Reset() = 0;
		};
	}
}