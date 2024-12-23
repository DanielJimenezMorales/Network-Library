#include "time_clock.h"

#include "Logger.h"

namespace NetLib
{
	TimeClock* TimeClock::_instance = nullptr;

	void TimeClock::CreateInstance()
	{
		if ( _instance == nullptr )
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
		if ( _instance != nullptr )
		{
			delete _instance;
			_instance = nullptr;
		}
	}

	uint64 TimeClock::GetLocalTimeMilliseconds() const
	{
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration< long long, std::milli > duration =
		    std::chrono::round< std::chrono::milliseconds >( currentTime - _startTime );
		return duration.count();
	}

	float64 TimeClock::GetLocalTimeSeconds() const
	{
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration< float64 > duration = currentTime - _startTime;
		return duration.count();
	}

	float64 TimeClock::GetServerTimeSeconds() const
	{
		return GetLocalTimeSeconds() + _serverClockTimeDeltaSeconds;
	}

	float64 TimeClock::GetElapsedTimeSeconds() const
	{
		std::chrono::duration< float64 > elapsedTimeSeconds = _elapsedTimeNanoseconds;
		return elapsedTimeSeconds.count();
	}

	void TimeClock::UpdateLocalTime()
	{
		std::chrono::time_point< std::chrono::steady_clock > current = std::chrono::steady_clock::now();
		_elapsedTimeNanoseconds = std::chrono::round< std::chrono::nanoseconds >( current - _lastTimeUpdate );

		_lastTimeUpdate = current;
	}

	void TimeClock::SetServerClockTimeDelta( float64 newValue )
	{
		LOG_INFO( "Adjusting Server's clock time delta. Old value: %f sec, New value: %f sec, Difference: %f sec",
		          _serverClockTimeDeltaSeconds, newValue, ( _serverClockTimeDeltaSeconds - newValue ) );

		_serverClockTimeDeltaSeconds = newValue;
	}

	TimeClock::TimeClock()
	    : _startTime( std::chrono::steady_clock::now() )
	    , _lastTimeUpdate( std::chrono::steady_clock::now() )
	    , _serverClockTimeDeltaSeconds( 0.0f )
	{
	}
} // namespace NetLib
