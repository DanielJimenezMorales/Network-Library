#include "packet_loss_metric.h"

#include "metrics/metric_names.h"

#include "logger.h"

namespace NetLib
{
	namespace Metrics
	{
		PacketLossMetric::PacketLossMetric()
		    : _updateRate( 1.f )
		    , _timeUntilNextUpdate( 1.f )
		    , _sentMessages( 0 )
		    , _lostMessages( 0 )
		    , _currentValue( 0 )
		    , _maxValue( 0 )
		{
		}

		void PacketLossMetric::GetName( std::string& out_name_buffer ) const
		{
			out_name_buffer.assign( PACKET_LOSS_METRIC );
		}

		uint32 PacketLossMetric::GetValue( const std::string& value_type ) const
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
				LOG_WARNING( "Unknown value type '%s' for PacketLossMetric", value_type.c_str() );
			}

			return result;
		}

		void PacketLossMetric::SetUpdateRate( float32 update_rate )
		{
			_updateRate = update_rate;
			_timeUntilNextUpdate = _updateRate;
		}

		void PacketLossMetric::Update( float32 elapsed_time )
		{
			if ( _timeUntilNextUpdate <= elapsed_time )
			{
				const float32 lostMessagesPercentage =
				    ( _sentMessages == 0 )
				        ? 0
				        : ( static_cast< float32 >( _lostMessages ) / static_cast< float32 >( _sentMessages ) ) * 100.f;
				_currentValue = lostMessagesPercentage;

				if ( _currentValue > _maxValue )
				{
					_maxValue = _currentValue;
				}

				LOG_WARNING( "PL LOST: %u, SENT: %u", _lostMessages, _sentMessages );
				_lostMessages = 0;
				_sentMessages = 0;

				_timeUntilNextUpdate = _updateRate;
			}
			else
			{
				_timeUntilNextUpdate -= elapsed_time;
			}
		}

		void PacketLossMetric::AddValueSample( uint32 value, const std::string& sample_type )
		{
			if ( sample_type == "SENT" )
			{
				++_sentMessages;
			}
			else if ( sample_type == "LOST" )
			{
				++_lostMessages;
			}
			else
			{
				LOG_WARNING( "Unknown sample type '%s' for PacketLossMetric", sample_type.c_str() );
			}
		}

		void PacketLossMetric::Reset()
		{
			_sentMessages = 0;
			_lostMessages = 0;
			_currentValue = 0;
			_maxValue = 0;
		}
	} // namespace Metrics
} // namespace NetLib
