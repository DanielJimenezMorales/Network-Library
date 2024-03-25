#pragma once
#include <chrono>
#include <cstdint>

namespace NetLib
{
	class TimeClock
	{
	public:
		static void CreateInstance();
		static TimeClock& GetInstance();
		static void DeleteInstance();

		uint64_t GetLocalTimeMilliseconds() const;
		double GetLocalTimeSeconds() const;
		double GetServerTimeSeconds() const;
		double GetElapsedTimeSeconds() const;

		void UpdateLocalTime();

		void SetServerClockTimeDelta(double newValue);

	private:
		TimeClock();
		TimeClock(const TimeClock&) = delete;
		TimeClock& operator=(const TimeClock&) = delete;

		static TimeClock* _instance;
		const std::chrono::time_point<std::chrono::steady_clock> _startTime;

		std::chrono::time_point<std::chrono::steady_clock> _lastTimeUpdate;
		std::chrono::duration<long long, std::nano> _elapsedTimeNanoseconds;

		double _serverClockTimeDeltaSeconds;
	};
}
