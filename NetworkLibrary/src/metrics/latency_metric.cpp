#include "latency_metric.h"

#include "logger.h"
#include "AlgorithmUtils.h"

namespace NetLib
{
	namespace Metrics
	{
		LatencyMetric::LatencyMetric()
			: _timeUntilNextUpdate(1.0f)
			, _updateRate(1.0f)
			, _currentValue(0)
			, _maxValue(0)
			, _samples()
		{
			_samples.reserve(MAX_BUFFER_SIZE);
		}

		void LatencyMetric::GetName(std::string& out_name_buffer) const
		{
			out_name_buffer.assign("LATENCY");
		}

		uint32 LatencyMetric::GetValue(const std::string& value_type) const
		{
			uint32 result = 0;

			if (value_type == "MAX")
			{
				result = _maxValue;
			}
			else if (value_type == "CURRENT")
			{
				result = _currentValue;
			}
			else
			{
				LOG_WARNING("Unknown value type '%s' for LatencyNetworkStatisticEntry", value_type.c_str());
			}

			return result;
		}

		void LatencyMetric::SetUpdateRate(float32 update_rate)
		{
			_updateRate = update_rate;
			_timeUntilNextUpdate = _updateRate;
		}

		void LatencyMetric::Update(float32 elapsed_time)
		{
			if (_timeUntilNextUpdate <= elapsed_time)
			{
				for (auto cit = _samples.cbegin(); cit != _samples.cend(); ++cit)
				{
					if (_currentValue == 0)
					{
						_currentValue = *cit;
					}
					else
					{
						_currentValue = Common::AlgorithmUtils::ExponentialMovingAverage(_currentValue, *cit, 10);
					}

					if (*cit > _maxValue)
					{
						_maxValue = *cit;
					}
				}

				_samples.clear();
				_timeUntilNextUpdate = _updateRate;
			}
			else
			{
				_timeUntilNextUpdate -= elapsed_time;
			}
		}

		void LatencyMetric::AddValueSample(uint32 value)
		{
			if (_samples.capacity() == _samples.size())
			{
				LOG_WARNING(
					"Latency samples vector is full and as a consequence latency stats might be inaccurate, consider "
					"increasing the buffer capacity");
			}
			else
			{
				_samples.push_back(value);
			}
		}

		void LatencyMetric::Reset()
		{
			_timeUntilNextUpdate = _updateRate;
			_currentValue = 0;
			_maxValue = 0;
			_samples.clear();
		}
	}
} // namespace NetLib
