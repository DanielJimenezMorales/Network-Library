#include "increment_metric.h"

#include "logger.h"

namespace NetLib
{
	namespace Metrics
	{
		IncrementMetric::IncrementMetric( MetricType type )
		    : _type( type )
		    , _currentValue( 0 )
		{
		}

		MetricType IncrementMetric::GetType() const
		{
			return _type;
		}

		uint32 IncrementMetric::GetValue( ValueType value_type ) const
		{
			uint32 result = 0;
			if ( value_type == ValueType::CURRENT )
			{
				result = _currentValue;
			}
			else
			{
				LOG_WARNING( "Unknown value type '%s' for %u Metric", static_cast< uint8 >( value_type ),
				             static_cast< uint8 >( _type ) );
			}

			return result;
		}

		void IncrementMetric::SetUpdateRate( float32 update_rate )
		{
		}

		void IncrementMetric::Update( float32 elapsed_time )
		{
		}

		void IncrementMetric::AddValueSample( uint32 value, const std::string& sample_type )
		{
			_currentValue += value;
		}

		void IncrementMetric::Reset()
		{
			_currentValue = 0;
		}
	} // namespace Metrics
} // namespace NetLib
