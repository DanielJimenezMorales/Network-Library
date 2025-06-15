#include "increment_metric.h"

#include "logger.h"

namespace NetLib
{
	namespace Metrics
	{
		IncrementMetric::IncrementMetric( const std::string& name )
		    : _name( name )
		    , _currentValue( 0 )
		{
		}

		void IncrementMetric::GetName( std::string& out_name_buffer ) const
		{
			out_name_buffer.assign( _name );
		}

		uint32 IncrementMetric::GetValue( const std::string& value_type ) const
		{
			uint32 result = 0;
			if ( value_type == "CURRENT" )
			{
				result = _currentValue;
			}
			else
			{
				LOG_WARNING( "Unknown value type '%s' for %s Metric", value_type.c_str(), _name.c_str() );
			}

			return result;
		}

		void IncrementMetric::SetUpdateRate( float32 update_rate )
		{
		}

		void IncrementMetric::Update( float32 elapsed_time )
		{
		}

		void IncrementMetric::AddValueSample( uint32 value )
		{
			_currentValue += value;
		}

		void IncrementMetric::Reset()
		{
			_currentValue = 0;
		}
	} // namespace Metrics
} // namespace NetLib
