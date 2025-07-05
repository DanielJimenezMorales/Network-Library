#pragma once
#include "metrics/i_metric.h"

#include <vector>

namespace NetLib
{
	namespace Metrics
	{
		class JitterMetric : public IMetric
		{
			public:
				static const uint32 MAX_BUFFER_SIZE = 100;
				static const uint32 JITTER_SMOOTHIN_FACTOR = 10;

				JitterMetric();

				void GetName( std::string& out_name_buffer ) const override;
				uint32 GetValue( const std::string& value_type ) const override;
				void SetUpdateRate( float32 update_rate ) override;
				void Update( float32 elapsed_time ) override;
				void AddValueSample( uint32 value, const std::string& sample_type = "NONE" ) override;
				void Reset() override;

			private:
				std::vector< uint32 > _latencySamples;
				uint32 _lastLatencySample;
				uint32 _currentValue;
				uint32 _maxValue;
		};
	}
} // namespace NetLib
