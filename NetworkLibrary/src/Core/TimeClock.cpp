#include "TimeClock.h"

TimeClock* TimeClock::_instance = nullptr;

void TimeClock::CreateInstance()
{
	if (_instance == nullptr)
	{
		_instance = new TimeClock();
	}
}

TimeClock& TimeClock::GetInstance()
{
	return *_instance;
}

void TimeClock::DeleteInstance()
{
	if (_instance != nullptr)
	{
		delete _instance;
		_instance = nullptr;
	}
}

uint64_t TimeClock::GetElapsedTimeSinceStartMilliseconds() const
{
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<long long, std::milli> duration = std::chrono::round<std::chrono::milliseconds>(currentTime - _startTime);
	return duration.count();
}

double TimeClock::GetElapsedTimeSeconds() const
{
	std::chrono::duration<double> elapsedTimeSeconds = _elapsedTimeNanoseconds;
	return elapsedTimeSeconds.count();
}

void TimeClock::UpdateLocalTime()
{
	std::chrono::time_point<std::chrono::steady_clock> current = std::chrono::steady_clock::now();
	_elapsedTimeNanoseconds = std::chrono::round<std::chrono::nanoseconds>(current - _lastTimeUpdate);

	_lastTimeUpdate = current;
}

TimeClock::TimeClock() : _startTime(std::chrono::steady_clock::now()), _lastTimeUpdate(std::chrono::steady_clock::now())
{
}
