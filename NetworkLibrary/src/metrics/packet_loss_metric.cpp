#include "packet_loss_metric.h"

#include "metrics/metric_types.h"

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

		MetricType PacketLossMetric::GetType() const
		{
			return MetricType::PACKET_LOSS;
		}

		uint32 PacketLossMetric::GetValue( ValueType value_type ) const
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
				LOG_WARNING( "Unknown value type '%u' for PacketLossMetric", static_cast< uint8 >( value_type ) );
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
				_currentValue = static_cast< uint32 >( ( lostMessagesPercentage ) );

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
