#pragma once
#include <chrono>
#include <cstdint>

class TimeClock
{
public:
	static void CreateInstance();
	static TimeClock& GetInstance();
	static void DeleteInstance();

	uint64_t GetElapsedTimeSinceStartMilliseconds() const;
	double GetElapsedTimeSeconds() const;

	void UpdateLocalTime();

private:
	TimeClock();

	static TimeClock* _instance;
	const std::chrono::time_point<std::chrono::steady_clock> _startTime;

	std::chrono::time_point<std::chrono::steady_clock> _lastTimeUpdate;
	std::chrono::duration<long long, std::nano> _elapsedTimeNanoseconds;
};

