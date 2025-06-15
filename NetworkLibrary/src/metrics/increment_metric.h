#pragma once
#include "metrics/i_metric.h"

namespace NetLib
{
	namespace Metrics
	{
		class IncrementMetric : public IMetric
		{
			public:
				IncrementMetric( const std::string& name );

				void GetName( std::string& out_name_buffer ) const override;
				uint32 GetValue( const std::string& value_type ) const override;
				void SetUpdateRate( float32 update_rate ) override;
				void Update( float32 elapsed_time ) override;
				void AddValueSample( uint32 value ) override;
				void Reset() override;

			private:
				uint32 _currentValue;
				const std::string _name;
		};
	}
}
