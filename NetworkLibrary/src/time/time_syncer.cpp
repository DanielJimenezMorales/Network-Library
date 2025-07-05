#include "time_syncer.h"

#include "logger.h"

#include "core/remote_peer.h"
#include "core/time_clock.h"

#include "communication/message_factory.h"

#include <memory>

namespace NetLib
{
	TimeSyncer::TimeSyncer()
	    : _requestFrequencySeconds( DEFAULT_TIME_REQUESTS_FREQUENCY_SECONDS )
	    , _timeSinceLastTimeRequest( 0.0f )
	    , _numberOfInitialTimeRequestBurstLeft( NUMBER_OF_INITIAL_TIME_REQUESTS_BURST )
	{
	}

	static std::unique_ptr< TimeRequestMessage > CreateTimeRequestMessage()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > lendMessage( messageFactory.LendMessage( MessageType::TimeRequest ) );

		std::unique_ptr< TimeRequestMessage > timeRequestMessage(
		    static_cast< TimeRequestMessage* >( lendMessage.release() ) );

		timeRequestMessage->SetOrdered( true );
		TimeClock& timeClock = TimeClock::GetInstance();
		timeRequestMessage->remoteTime = timeClock.GetLocalTimeMilliseconds();

		LOG_INFO( "TIME REQUEST CREATED" );
		return std::move( timeRequestMessage );
	}

	void TimeSyncer::Update( float32 elapsed_time, RemotePeer& remote_peer )
	{
		_timeSinceLastTimeRequest += elapsed_time;

		if ( IsSyncNeeded() )
		{
			if ( IsInitialBurstActive() )
			{
				--_numberOfInitialTimeRequestBurstLeft;
			}

			std::unique_ptr< TimeRequestMessage > timeRequestMessage = CreateTimeRequestMessage();
			remote_peer.AddMessage( std::move( timeRequestMessage ) );
		}
	}

	void TimeSyncer::ProcessTimeResponse( const TimeResponseMessage& message )
	{
		LOG_INFO( "PROCESSING TIME RESPONSE" );

		// Add new RTT to buffer
		TimeClock& timeClock = TimeClock::GetInstance();
		const uint32 rtt = timeClock.GetLocalTimeMilliseconds() - message.remoteTime;
		_timeRequestRTTs.push_back( rtt );

		// If the buffer is full, remove the oldest RTT
		if ( _timeRequestRTTs.size() == TIME_REQUEST_RTT_BUFFER_SIZE + 1 )
		{
			_timeRequestRTTs.pop_front();
		}

		// Get RTT to adjust server's clock elapsed time
		uint32 meanRTT = 0;
		if ( _timeRequestRTTs.size() == TIME_REQUEST_RTT_BUFFER_SIZE )
		{
			// Sort RTTs and remove the smallest and biggest values (They are considered outliers!)
			std::list< uint32 > sortedTimeRequestRTTs = _timeRequestRTTs;
			sortedTimeRequestRTTs.sort();

			// Remove potential outliers
			for ( uint32 i = 0; i < NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE; ++i )
			{
				sortedTimeRequestRTTs.pop_back();
				sortedTimeRequestRTTs.pop_front();
			}

			std::list< uint32 >::const_iterator cit = sortedTimeRequestRTTs.cbegin();
			for ( ; cit != sortedTimeRequestRTTs.cend(); ++cit )
			{
				meanRTT += *cit;
			}

			const uint32 NUMBER_OF_VALID_RTT_TO_AVERAGE =
			    TIME_REQUEST_RTT_BUFFER_SIZE - ( 2 * NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE );

			meanRTT /= NUMBER_OF_VALID_RTT_TO_AVERAGE;
		}
		else
		{
			meanRTT = rtt;
		}

		// Calculate server clock delta time
		const uint32 serverClockElapsedTimeMilliseconds = message.serverTime - message.remoteTime - ( meanRTT / 2 );
		const float64 serverClockElapsedTimeSeconds =
		    static_cast< float64 >( serverClockElapsedTimeMilliseconds ) / 1000;
		timeClock.SetServerClockTimeDelta( serverClockElapsedTimeSeconds );

		LOG_INFO( "SERVER TIME UPDATED. Local time: %f sec, Server time: %f sec", timeClock.GetLocalTimeSeconds(),
		          timeClock.GetServerTimeSeconds() );
	}

	bool TimeSyncer::IsInitialBurstActive() const
	{
		return _numberOfInitialTimeRequestBurstLeft > 0;
	}

	bool TimeSyncer::IsSyncNeeded() const
	{
		bool result = false;
		if ( IsInitialBurstActive() || _timeSinceLastTimeRequest >= _requestFrequencySeconds )
		{
			result = true;
		}

		return result;
	}
} // namespace NetLib
