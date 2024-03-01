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

uint64_t TimeClock::GetElapsedTimeInMilliseconds() const
{
	auto currentTime = std::chrono::steady_clock::now();
	auto duration = currentTime - _startTime;
	uint64_t durationInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return durationInMilliseconds;
}

TimeClock::TimeClock() : _startTime(std::chrono::steady_clock::now())
{
}
