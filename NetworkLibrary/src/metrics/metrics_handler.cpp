#include "metrics_handler.h"

#include "AlgorithmUtils.h"
#include "logger.h"

#include "metrics/metric_names.h"
#include "metrics/latency_metric.h"
#include "metrics/jitter_metric.h"
#include "metrics/upload_bandwidth_metric.h"
#include "metrics/download_bandwidth_metric.h"
#include "metrics/increment_metric.h"

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
			AddEntry( std::make_unique< DownloadBandwidthMetric >() );
			AddEntry( std::make_unique< IncrementMetric >( DUPLICATE_METRIC ) );
			AddEntry( std::make_unique< IncrementMetric >( OUT_OF_ORDER_METRIC ) );
			AddEntry( std::make_unique< IncrementMetric >( RETRANSMISSION_METRIC ) );
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

			LOG_INFO( "NETWORK METRICS:\nLATENCY: Average: %u, Max: %u\nJITTER: Average: %u, Max: %u\nUPLOAD "
			          "BANDWIDTH: Current: %u, "
			          "Max: %u\nDOWNLOAD BANDWIDTH: Current: %u, Max: %u\nRETRANSMISSIONS: Current: %u\nOUT OF ORDER: "
			          "Current: %u\nDUPLICATE: Current: %u",
			          GetValue( LATENCY_METRIC, CURRENT_VALUE_TYPE ), GetValue( LATENCY_METRIC, MAX_VALUE_TYPE ),
			          GetValue( JITTER_METRIC, CURRENT_VALUE_TYPE ), GetValue( JITTER_METRIC, MAX_VALUE_TYPE ),
			          GetValue( UPLOAD_BANDWIDTH_METRIC, CURRENT_VALUE_TYPE ),
			          GetValue( UPLOAD_BANDWIDTH_METRIC, MAX_VALUE_TYPE ),
			          GetValue( DOWNLOAD_BANDWIDTH_METRIC, CURRENT_VALUE_TYPE ),
			          GetValue( DOWNLOAD_BANDWIDTH_METRIC, MAX_VALUE_TYPE ),
			          GetValue( RETRANSMISSION_METRIC, CURRENT_VALUE_TYPE ),
			          GetValue( OUT_OF_ORDER_METRIC, CURRENT_VALUE_TYPE ),
			          GetValue( DUPLICATE_METRIC, CURRENT_VALUE_TYPE ) );
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
				LOG_WARNING( "Can't get a value from a metric that doesn't exist. Name: '%s'", entry_name.c_str() );
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
				LOG_WARNING( "Can't add a value to a metric that doesn't exist. Name: '%s'", entry_name.c_str() );
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
