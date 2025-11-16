#include "bandwidth_metric.h"

#include "logger.h"

namespace NetLib
{
	namespace Metrics
	{
		BandwidthMetric::BandwidthMetric()
		    : _updateRate( 1.0f )
		    , _timeUntilNextUpdate( 1.0f )
		    , _inProgressValue( 0 )
		    , _currentValue( 0 )
		    , _maxValue( 0 )
		{
		}

		uint32 BandwidthMetric::GetValue( const std::string& value_type ) const
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
				LOG_WARNING( "Unknown value type '%s' for BandwidthMetric", value_type.c_str() );
			}

			return result;
		}

		void BandwidthMetric::SetUpdateRate( float32 update_rate )
		{
			_updateRate = update_rate;
			_timeUntilNextUpdate = _updateRate;
		}

		void BandwidthMetric::Update( float32 elapsed_time )
		{
			if ( _timeUntilNextUpdate <= elapsed_time )
			{
				// Update current
				_currentValue = _inProgressValue;
				_inProgressValue = 0;

				// Update max
				if ( _currentValue > _maxValue )
				{
					_maxValue = _currentValue;
				}

				_timeUntilNextUpdate = _updateRate;
			}
			else
			{
				_timeUntilNextUpdate -= elapsed_time;
			}
		}

		void BandwidthMetric::AddValueSample( uint32 value, const std::string& sample_type )
		{
			_inProgressValue += value;
		}

		void BandwidthMetric::Reset()
		{
			_inProgressValue = 0;
			_currentValue = 0;
			_maxValue = 0;
			_timeUntilNextUpdate = _updateRate;
		}
	} // namespace Metrics
} // namespace NetLib
