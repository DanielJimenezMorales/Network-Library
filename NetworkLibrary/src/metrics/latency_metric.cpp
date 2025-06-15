#include "latency_metric.h"

#include "logger.h"
#include "AlgorithmUtils.h"

#include "metrics/metric_names.h"

namespace NetLib
{
	namespace Metrics
	{
		LatencyMetric::LatencyMetric()
		    : _currentValue( 0 )
		    , _maxValue( 0 )
		    , _samples()
		{
			_samples.reserve( MAX_BUFFER_SIZE );
		}

		void LatencyMetric::GetName( std::string& out_name_buffer ) const
		{
			out_name_buffer.assign( LATENCY_METRIC );
		}

		uint32 LatencyMetric::GetValue( const std::string& value_type ) const
		{
			uint32 result = 0;

			if ( value_type == "MAX" )
			{
				result = _maxValue;
			}
			else if ( value_type == "CURRENT" )
			{
				result = _currentValue;
			}
			else
			{
				LOG_WARNING( "Unknown value type '%s' for LatencyMetric", value_type.c_str() );
			}

			return result;
		}

		void LatencyMetric::SetUpdateRate( float32 update_rate )
		{
		}

		void LatencyMetric::Update( float32 elapsed_time )
		{
			for ( auto cit = _samples.cbegin(); cit != _samples.cend(); ++cit )
			{
				if ( _currentValue == 0 )
				{
					_currentValue = *cit;
				}
				else
				{
					_currentValue = Common::AlgorithmUtils::ExponentialMovingAverage( _currentValue, *cit,
					                                                                  LATENCY_SMOOTHIN_FACTOR );
				}

				if ( *cit > _maxValue )
				{
					_maxValue = *cit;
				}
			}

			_samples.clear();
		}

		void LatencyMetric::AddValueSample( uint32 value )
		{
			if ( _samples.capacity() == _samples.size() )
			{
				LOG_WARNING(
				    "Latency samples vector is full and as a consequence latency stats might be inaccurate, consider "
				    "increasing the buffer capacity" );
			}
			else
			{
				_samples.push_back( value );
			}
		}

		void LatencyMetric::Reset()
		{
			_currentValue = 0;
			_maxValue = 0;
			_samples.clear();
		}
	} // namespace Metrics
} // namespace NetLib
