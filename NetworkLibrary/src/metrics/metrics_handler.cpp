#include "metrics_handler.h"

#include "AlgorithmUtils.h"
#include "logger.h"
#include "asserts.h"

#include "metrics/latency_metric.h"
#include "metrics/packet_loss_metric.h"
#include "metrics/jitter_metric.h"
#include "metrics/upload_bandwidth_metric.h"
#include "metrics/download_bandwidth_metric.h"
#include "metrics/increment_metric.h"

namespace NetLib
{
	namespace Metrics
	{
		const std::vector< MetricType > MetricsHandler::ALL_METRICS = { MetricType::LATENCY,
		                                                                MetricType::JITTER,
		                                                                MetricType::PACKET_LOSS,
		                                                                MetricType::UPLOAD_BANDWIDTH,
		                                                                MetricType::DOWNLOAD_BANDWIDTH,
		                                                                MetricType::RETRANSMISSIONS,
		                                                                MetricType::OUT_OF_ORDER_MESSAGES,
		                                                                MetricType::DUPLICATE_MESSAGES };

		MetricsHandler::MetricsHandler()
		    : _isStartedUp( false )
		    , _entries()
		{
		}

		MetricsHandler::~MetricsHandler()
		{
			ASSERT( !_isStartedUp, "[MetricsHandler.%s] Before destructing instance, call ShutDown.",
			        THIS_FUNCTION_NAME );
		}

		bool MetricsHandler::AddMetrics( float32 update_rate, const std::vector< MetricType >& metrics )
		{
			bool result = true;

			for ( auto cit = metrics.cbegin(); cit != metrics.cend(); ++cit )
			{
				if ( HasMetric( *cit ) )
				{
					LOG_WARNING( "[MetricsHandler.%s] Metric of type %u already exists. Ignoring it.",
					             THIS_FUNCTION_NAME, static_cast< uint8 >( *cit ) );
					continue;
				}

				switch ( *cit )
				{
					case MetricType::LATENCY:
						result &= AddEntry( new LatencyMetric() );
						break;
					case MetricType::JITTER:
						result &= AddEntry( new JitterMetric() );
						break;
					case MetricType::PACKET_LOSS:
						result &= AddEntry( new PacketLossMetric() );
						break;
					case MetricType::UPLOAD_BANDWIDTH:
						result &= AddEntry( new UploadBandwidthMetric() );
						break;
					case MetricType::DOWNLOAD_BANDWIDTH:
						result &= AddEntry( new DownloadBandwidthMetric() );
						break;
					case MetricType::RETRANSMISSIONS:
					case MetricType::OUT_OF_ORDER_MESSAGES:
					case MetricType::DUPLICATE_MESSAGES:
						result &= AddEntry( new IncrementMetric( *cit ) );
						break;
					default:
						LOG_ERROR( "[MetricsHandler.%s] Unknown MetricType %u. Ignoring it.", THIS_FUNCTION_NAME,
						           static_cast< uint8 >( *cit ) );
						break;
				}
			}

			for ( auto it = _entries.begin(); it != _entries.end(); ++it )
			{
				it->second->SetUpdateRate( update_rate );
			}

			return result;
		}

		bool MetricsHandler::HasMetric( MetricType type ) const
		{
			return _entries.find( type ) != _entries.end();
		}

		bool MetricsHandler::StartUp( float32 update_rate, MetricsEnableConfig enable_config,
		                              const std::vector< MetricType >& enabled_metrics )
		{
			if ( _isStartedUp )
			{
				LOG_ERROR( "[MetricsHandler.%s] MetricsHandler is already started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			if ( enable_config == MetricsEnableConfig::ENABLE_ALL )
			{
				AddMetrics( update_rate, ALL_METRICS );
			}
			else if ( enable_config == MetricsEnableConfig::CUSTOM )
			{
				AddMetrics( update_rate, enabled_metrics );
			}

			_isStartedUp = true;
			return true;
		}

		bool MetricsHandler::ShutDown()
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[MetricsHandler.%s] MetricsHandler is not started up, ignoring call", THIS_FUNCTION_NAME );
				return false;
			}

			for ( auto it = _entries.begin(); it != _entries.end(); ++it )
			{
				delete it->second;
			}

			_entries.clear();
			_isStartedUp = false;
			return true;
		}

		void MetricsHandler::Update( float32 elapsed_time )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[MetricsHandler.%s] MetricsHandler is not started up, ignoring call", THIS_FUNCTION_NAME );
				return;
			}

			for ( auto it = _entries.begin(); it != _entries.end(); ++it )
			{
				it->second->Update( elapsed_time );
			}

			LOG_INFO(
			    "NETWORK METRICS:\nLATENCY: Average: %u, Max: %u\nJITTER: Average: %u, Max: %u\nPACKET LOSS: Average: "
			    "%u, Max: %u\nUPLOAD "
			    "BANDWIDTH: Current: %u, "
			    "Max: %u\nDOWNLOAD BANDWIDTH: Current: %u, Max: %u\nRETRANSMISSIONS: Current: %u\nOUT OF ORDER: "
			    "Current: %u\nDUPLICATE: Current: %u",
			    GetValue( MetricType::LATENCY, ValueType::CURRENT ), GetValue( MetricType::LATENCY, ValueType::MAX ),
			    GetValue( MetricType::JITTER, ValueType::CURRENT ), GetValue( MetricType::JITTER, ValueType::MAX ),
			    GetValue( MetricType::PACKET_LOSS, ValueType::CURRENT ),
			    GetValue( MetricType::PACKET_LOSS, ValueType::MAX ),
			    GetValue( MetricType::UPLOAD_BANDWIDTH, ValueType::CURRENT ),
			    GetValue( MetricType::UPLOAD_BANDWIDTH, ValueType::MAX ),
			    GetValue( MetricType::DOWNLOAD_BANDWIDTH, ValueType::CURRENT ),
			    GetValue( MetricType::DOWNLOAD_BANDWIDTH, ValueType::MAX ),
			    GetValue( MetricType::RETRANSMISSIONS, ValueType::CURRENT ),
			    GetValue( MetricType::OUT_OF_ORDER_MESSAGES, ValueType::CURRENT ),
			    GetValue( MetricType::DUPLICATE_MESSAGES, ValueType::CURRENT ) );
		}

		bool MetricsHandler::AddEntry( IMetric* metric )
		{
			ASSERT( metric != nullptr, "[MetricsHandler.%s] entry is nullptr.", THIS_FUNCTION_NAME );

			bool result = false;

			const MetricType metricType = metric->GetType();
			if ( !HasMetric( metricType ) )
			{
				_entries[ metricType ] = metric;
				result = true;
			}
			else
			{
				LOG_WARNING( "Network statistic metric of type '%u' already exists, ignoring the new one",
				             static_cast< uint8 >( metricType ) );
			}

			return result;
		}

		uint32 MetricsHandler::GetValue( MetricType metric_type, ValueType value_type ) const
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[MetricsHandler.%s] MetricsHandler is not started up, ignoring call", THIS_FUNCTION_NAME );
				return 0;
			}

			uint32 result = 0;

			auto it = _entries.find( metric_type );
			if ( it != _entries.end() )
			{
				result = it->second->GetValue( value_type );
			}
			else
			{
				LOG_WARNING( "Can't get a value from a metric that doesn't exist. Metric type: '%u'",
				             static_cast< uint8 >( metric_type ) );
			}

			return result;
		}

		bool MetricsHandler::AddValue( MetricType metric_type, uint32 value, const std::string& sample_type )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[MetricsHandler.%s] MetricsHandler is not started up, ignoring call", THIS_FUNCTION_NAME );
				return false;
			}

			bool result = false;

			auto it = _entries.find( metric_type );
			if ( it != _entries.end() )
			{
				it->second->AddValueSample( value, sample_type );
				result = true;
			}
			else
			{
				LOG_WARNING( "Can't add a value to a metric that doesn't exist. Metric type: '%u'",
				             static_cast< uint8 >( metric_type ) );
			}

			return result;
		}
	} // namespace Metrics
} // namespace NetLib
