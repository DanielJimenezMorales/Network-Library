#pragma once

namespace NetLib
{
	namespace Metrics
	{
		// Metric names
		static constexpr char* LATENCY_METRIC = "LATENCY";
		static constexpr char* JITTER_METRIC = "JITTER";
		static constexpr char* UPLOAD_BANDWIDTH_METRIC = "UPLOAD_BANDWIDTH";
		static constexpr char* OUT_OF_ORDER_METRIC = "OUT_OF_ORDER";
		static constexpr char* DUPLICATE_METRIC = "DUPLICATE";
		static constexpr char* RETRANSMISSION_METRIC = "RETRANSMISSION";

		// Metric value types
		static constexpr char* CURRENT_VALUE_TYPE = "CURRENT";
		static constexpr char* MAX_VALUE_TYPE = "MAX";
	}
}
