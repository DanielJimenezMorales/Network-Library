#include "retransmission_metric.h"

#include "logger.h"

namespace NetLib
{
	namespace Metrics
	{
		RetransmissionMetric::RetransmissionMetric()
		    : _currentValue( 0 )
		{
		}

		void RetransmissionMetric::GetName( std::string& out_name_buffer ) const
		{
			out_name_buffer.assign( "RETRANSMISSION" );
		}

		uint32 RetransmissionMetric::GetValue( const std::string& value_type ) const
		{
			uint32 result = 0;
			if ( value_type == "CURRENT" )
			{
				result = _currentValue;
			}
			else
			{
				LOG_WARNING( "Unknown value type '%s' for RetransmissionMetric", value_type.c_str() );
			}

			return result;
		}

		void RetransmissionMetric::SetUpdateRate( float32 update_rate )
		{
		}

		void RetransmissionMetric::Update( float32 elapsed_time )
		{
		}

		void RetransmissionMetric::AddValueSample( uint32 value )
		{
			_currentValue += value;
		}

		void RetransmissionMetric::Reset()
		{
			_currentValue = 0;
		}
	} // namespace Metrics
} // namespace NetLib
