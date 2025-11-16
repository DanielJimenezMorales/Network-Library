#include "upload_bandwidth_metric.h"

#include "metrics/metric_names.h"

namespace NetLib
{
	namespace Metrics
	{
		void UploadBandwidthMetric::GetName( std::string& out_name_buffer ) const
		{
			out_name_buffer.assign( UPLOAD_BANDWIDTH_METRIC );
		}
	} // namespace Metrics
} // namespace NetLib
