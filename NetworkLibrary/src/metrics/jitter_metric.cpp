#include "jitter_metric.h"

#include "logger.h"
#include "AlgorithmUtils.h"

#include "metrics/metric_names.h"

namespace NetLib
{
	namespace Metrics
	{
		JitterMetric::JitterMetric()
		    : _lastLatencySample( 0 )
		    , _currentValue( 0 )
		    , _maxValue( 0 )
		    , _latencySamples()
		{
			_latencySamples.reserve( MAX_BUFFER_SIZE );
		}

		void JitterMetric::GetName( std::string& out_name_buffer ) const
		{
			out_name_buffer.assign( JITTER_METRIC );
		}

		uint32 JitterMetric::GetValue( const std::string& value_type ) const
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
				LOG_WARNING( "Unknown value type '%s' for JitterMetric", value_type.c_str() );
			}

			return result;
		}

		void JitterMetric::SetUpdateRate( float32 update_rate )
		{
		}

		void JitterMetric::Update( float32 elapsed_time )
		{
			for ( auto cit = _latencySamples.cbegin(); cit != _latencySamples.cend(); ++cit )
			{
				if ( _lastLatencySample == 0 )
				{
					_lastLatencySample = *cit;
					continue;
				}

				const uint32 jitter_sample = std::abs( ( int32 ) _lastLatencySample - ( int32 ) *cit );
				if ( _currentValue == 0 )
				{
					_currentValue = jitter_sample;
				}
				else
				{
					_currentValue = Common::AlgorithmUtils::ExponentialMovingAverage( _currentValue, jitter_sample,
					                                                                  JITTER_SMOOTHIN_FACTOR );
				}

				if ( jitter_sample > _maxValue )
				{
					_maxValue = jitter_sample;
				}
			}
		}

		void JitterMetric::AddValueSample( uint32 value )
		{
			_latencySamples.push_back( value );
		}

		void JitterMetric::Reset()
		{
			_lastLatencySample = 0;
			_currentValue = 0;
			_maxValue = 0;
			_latencySamples.clear();
		}
	} // namespace Metrics
} // namespace NetLib