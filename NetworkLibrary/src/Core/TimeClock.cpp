#include <sstream>

#include "TimeClock.h"
#include "Logger.h"

namespace NetLib
{
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

	uint64_t TimeClock::GetLocalTimeMilliseconds() const
	{
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<long long, std::milli> duration = std::chrono::round<std::chrono::milliseconds>(currentTime - _startTime);
		return duration.count();
	}

	double TimeClock::GetLocalTimeSeconds() const
	{
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<double> duration = currentTime - _startTime;
		return duration.count();
	}

	double TimeClock::GetServerTimeSeconds() const
	{
		return GetLocalTimeSeconds() + _serverClockTimeDeltaSeconds;
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

	void TimeClock::SetServerClockTimeDelta(double newValue)
	{
		std::stringstream ss;
		ss << "Adjusting Server's clock time delta. Old value: " << _serverClockTimeDeltaSeconds << "s, New value: " << newValue << "s, Difference: " << _serverClockTimeDeltaSeconds - newValue << "s";
		Common::LOG_INFO(ss.str());

		_serverClockTimeDeltaSeconds = newValue;
	}

	TimeClock::TimeClock() : _startTime(std::chrono::steady_clock::now()), _lastTimeUpdate(std::chrono::steady_clock::now()), _serverClockTimeDeltaSeconds(0.0f)
	{
	}
}
