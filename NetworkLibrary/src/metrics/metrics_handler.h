#pragma once
#include "numeric_types.h"

#include "metrics/i_metric.h"
#include "metrics/metric_types.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

namespace NetLib
{
	namespace Metrics
	{
		enum class MetricsEnableConfig : uint8
		{
			ENABLE_ALL = 0,
			DISABLE_ALL = 1,
			CUSTOM = 2
		};

		class MetricsHandler
		{
			public:
				MetricsHandler();
				~MetricsHandler();

				bool StartUp( float32 update_rate, MetricsEnableConfig enable_config,
				              const std::vector< MetricType >& enabled_metrics = {} );
				bool ShutDown();

				void Update( float32 elapsed_time );

				/// <summary>
				/// Gets the value of type value_type (MAX, CURRENT...) from the metric of type metric_type. If the
				/// 1) Metrics handler is not started up, 2) The metric does not exist or 3) the value type is invalid
				/// the function returns 0.
				/// </summary>
				uint32 GetValue( MetricType metric_type, ValueType value_type ) const;

				/// <summary>
				/// Adds a value to the metric of type metric_type. If the 1) Metrics handler is not started up or 2)
				/// the metric doesn't exists it does nothing and returns false.
				/// </summary>
				bool AddValue( MetricType metric_type, uint32 value, const std::string& sample_type = "NONE" );

				bool HasMetric( MetricType type ) const;

			private:
				bool AddMetrics( float32 update_rate, const std::vector< MetricType >& metrics );
				bool AddEntry( IMetric* metric );

				bool _isStartedUp;
				std::unordered_map< MetricType, IMetric* > _entries;

				static const std::vector< MetricType > ALL_METRICS;
		};
	} // namespace Metrics
} // namespace NetLib