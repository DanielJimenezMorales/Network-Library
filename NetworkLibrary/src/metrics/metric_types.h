#pragma once

namespace NetLib
{
	namespace Metrics
	{
		enum class MetricType : uint8
		{
			LATENCY = 0,
			JITTER = 1,
			PACKET_LOSS = 2,
			UPLOAD_BANDWIDTH = 3,
			DOWNLOAD_BANDWIDTH = 4,
			RETRANSMISSIONS = 5,
			OUT_OF_ORDER_MESSAGES = 6,
			DUPLICATE_MESSAGES = 7
		};

		enum class ValueType : uint8
		{
			CURRENT = 0,
			MAX = 1
		};
	}
}
