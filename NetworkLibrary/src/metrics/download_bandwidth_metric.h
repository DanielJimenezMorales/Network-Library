#pragma once
#include "metrics/i_metric.h"

namespace NetLib
{
	namespace Metrics
	{
		class DownloadBandwidthMetric : public IMetric
		{
			public:
				DownloadBandwidthMetric();

				MetricType GetType() const override;
				uint32 GetValue( ValueType value_type ) const override;
				void SetUpdateRate( float32 update_rate ) override;
				void Update( float32 elapsed_time ) override;
				void AddValueSample( uint32 value, const std::string& sample_type = "NONE" ) override;
				void Reset() override;

			private:
				float32 _timeUntilNextUpdate;
				float32 _updateRate;

				uint32 _inProgressValue;
				uint32 _currentValue;
				uint32 _maxValue;
		};
	}
}
