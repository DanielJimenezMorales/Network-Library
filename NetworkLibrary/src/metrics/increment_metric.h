#pragma once
#include "metrics/i_metric.h"

#include "metrics/metric_types.h"

namespace NetLib
{
	namespace Metrics
	{
		class IncrementMetric : public IMetric
		{
			public:
				IncrementMetric( MetricType type );

				MetricType GetType() const override;
				uint32 GetValue( ValueType value_type ) const override;
				void SetUpdateRate( float32 update_rate ) override;
				void Update( float32 elapsed_time ) override;
				void AddValueSample( uint32 value, const std::string& sample_type = "NONE" ) override;
				void Reset() override;

			private:
				uint32 _currentValue;
				const MetricType _type;
		};
	}
}
