#pragma once
#include "metrics/bandwidth_metric.h"

namespace NetLib
{
	namespace Metrics
	{
		class UploadBandwidthMetric : public BandwidthMetric
		{
			public:
				void GetName( std::string& out_name_buffer ) const override;
		};
	}
}
