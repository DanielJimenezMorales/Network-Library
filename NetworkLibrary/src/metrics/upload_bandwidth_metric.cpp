#include "upload_bandwidth_metric.h"

#include "logger.h"

#include "metrics/metric_names.h"

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

		void UploadBandwidthMetric::GetName( std::string& out_name_buffer ) const
		{
			out_name_buffer.assign( UPLOAD_BANDWIDTH_METRIC );
		}

		uint32 UploadBandwidthMetric::GetValue( const std::string& value_type ) const
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
				LOG_WARNING( "Unknown value type '%s' for UploadBandwidthMetric", value_type.c_str() );
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
