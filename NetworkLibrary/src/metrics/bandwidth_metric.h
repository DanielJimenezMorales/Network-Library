#pragma once
#include "metrics/i_metric.h"

namespace NetLib
{
	namespace Metrics
	{
		/// <summary>
		/// Base class for bandwidth metrics (upload and download).
		/// Tracks current bandwidth usage and maximum bandwidth over time.
		/// </summary>
		class BandwidthMetric : public IMetric
		{
			public:
				BandwidthMetric();

				uint32 GetValue( const std::string& value_type ) const override;
				void SetUpdateRate( float32 update_rate ) override;
				void Update( float32 elapsed_time ) override;
				void AddValueSample( uint32 value, const std::string& sample_type = "NONE" ) override;
				void Reset() override;

			protected:
				float32 _timeUntilNextUpdate;
				float32 _updateRate;

				uint32 _inProgressValue;
				uint32 _currentValue;
				uint32 _maxValue;
		};
	}
}
