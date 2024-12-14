#pragma once
#include <chrono>

#include "NumericTypes.h"

namespace NetLib
{
	class TimeClock
	{
	public:
		static void CreateInstance();
		static TimeClock& GetInstance();
		static void DeleteInstance();

		uint64 GetLocalTimeMilliseconds() const;
		float64 GetLocalTimeSeconds() const;
		float64 GetServerTimeSeconds() const;
		float64 GetElapsedTimeSeconds() const;

		void UpdateLocalTime();

		void SetServerClockTimeDelta(float64 newValue);

	private:
		TimeClock();
		TimeClock(const TimeClock&) = delete;
		TimeClock& operator=(const TimeClock&) = delete;

		static TimeClock* _instance;
		const std::chrono::time_point<std::chrono::steady_clock> _startTime;

		std::chrono::time_point<std::chrono::steady_clock> _lastTimeUpdate;
		std::chrono::duration<long long, std::nano> _elapsedTimeNanoseconds;

		float64 _serverClockTimeDeltaSeconds;
	};
}
