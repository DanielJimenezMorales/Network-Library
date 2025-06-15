#include "metrics_handler.h"

#include "AlgorithmUtils.h"
#include "logger.h"

#include "metrics/latency_metric.h"
#include "metrics/jitter_metric.h"
#include "metrics/upload_bandwidth_metric.h"
#include "metrics/retransmission_metric.h"

#include <cassert>

namespace NetLib
{
	namespace Metrics
	{
		MetricsHandler::MetricsHandler()
		    : _entries()
		{
			AddEntry( std::make_unique< LatencyMetric >() );
			AddEntry( std::make_unique< JitterMetric >() );
			AddEntry( std::make_unique< UploadBandwidthMetric >() );
			AddEntry( std::make_unique< RetransmissionMetric >() );
		}

		void MetricsHandler::Configure( float32 update_rate )
		{
			Reset();

			for ( auto it = _entries.begin(); it != _entries.end(); ++it )
			{
				it->second->SetUpdateRate( update_rate );
			}
		}

		void MetricsHandler::Update( float32 elapsed_time )
		{
			for ( auto it = _entries.begin(); it != _entries.end(); ++it )
			{
				it->second->Update( elapsed_time );
			}

			LOG_INFO( "LATENCY: Average: %u, Max: %u\nJITTER: Average: %u, Max: %u\nUPLOAD BANDWIDTH: Current: %u, "
			          "Max: %u\nRETRANSMISSIONS: Current: %u",
			          GetValue( "LATENCY", "CURRENT" ), GetValue( "LATENCY", "MAX" ), GetValue( "JITTER", "CURRENT" ),
			          GetValue( "JITTER", "MAX" ), GetValue( "UPLOAD_BANDWIDTH", "CURRENT" ),
			          GetValue( "UPLOAD_BANDWIDTH", "MAX" ), GetValue( "RETRANSMISSION", "CURRENT" ) );
		}

		bool MetricsHandler::AddEntry( std::unique_ptr< IMetric > entry )
		{
			assert( entry != nullptr );

			bool result = false;

			std::string name;
			entry->GetName( name );
			if ( _entries.find( name ) == _entries.end() )
			{
				_entries[ name ] = std::move( entry );
				result = true;
			}
			else
			{
				LOG_WARNING( "Network statistic entry with name '%s' already exists, ignoring the new one",
				             name.c_str() );
			}

			return result;
		}

		uint32 MetricsHandler::GetValue( const std::string& entry_name, const std::string& value_type ) const
		{
			uint32 result = 0;

			auto it = _entries.find( entry_name );
			if ( it != _entries.end() )
			{
				result = it->second->GetValue( value_type );
			}
			else
			{
				LOG_WARNING( "Network statistic entry with name '%s' not found", entry_name.c_str() );
			}

			return result;
		}

		bool MetricsHandler::AddValue( const std::string& entry_name, uint32 value )
		{
			bool result = false;

			auto it = _entries.find( entry_name );
			if ( it != _entries.end() )
			{
				it->second->AddValueSample( value );
				result = true;
			}
			else
			{
				LOG_WARNING( "Network statistic entry with name '%s' not found", entry_name.c_str() );
			}

			return result;
		}

		void MetricsHandler::Reset()
		{
			for ( auto it = _entries.begin(); it != _entries.end(); ++it )
			{
				it->second->Reset();
			}
		}
	} // namespace Metrics
} // namespace NetLib
