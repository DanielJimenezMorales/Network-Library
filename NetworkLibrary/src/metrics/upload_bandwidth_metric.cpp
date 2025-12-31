#include "upload_bandwidth_metric.h"

#include "logger.h"

namespace NetLib
{
	namespace Metrics
	{
		UploadBandwidthMetric::UploadBandwidthMetric()
		    : _updateRate( 1.0f )
		    , _timeUntilNextUpdate( 1.0f )
		    , _inProgressValue( 0 )
		    , _currentValue( 0 )
		    , _maxValue( 0 )
		{
		}

		MetricType UploadBandwidthMetric::GetType() const
		{
			return MetricType::UPLOAD_BANDWIDTH;
		}

		uint32 UploadBandwidthMetric::GetValue( ValueType value_type ) const
		{
			uint32 result = 0;

			if ( value_type == ValueType::MAX )
			{
				result = _maxValue;
			}
			else if ( value_type == ValueType::CURRENT )
			{
				result = _currentValue;
			}
			else
			{
				LOG_WARNING( "Unknown value type '%u' for UploadBandwidthMetric", static_cast< uint8 >( value_type ) );
			}

			return result;
		}

		void UploadBandwidthMetric::SetUpdateRate( float32 update_rate )
		{
			_updateRate = update_rate;
			_timeUntilNextUpdate = _updateRate;
		}

		void UploadBandwidthMetric::Update( float32 elapsed_time )
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

		void UploadBandwidthMetric::AddValueSample( uint32 value, const std::string& sample_type )
		{
			_inProgressValue += value;
		}

		void UploadBandwidthMetric::Reset()
		{
			_inProgressValue = 0;
			_currentValue = 0;
			_maxValue = 0;
			_timeUntilNextUpdate = _updateRate;
		}
	} // namespace Metrics
} // namespace NetLib
