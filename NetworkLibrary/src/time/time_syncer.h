#pragma once
#include "numeric_types.h"

#include <list>

namespace NetLib
{
	class RemotePeer;
	class MessageFactory;
	class TimeResponseMessage;

	// This will discard the X biggest and smallest RTTs from the Adjusted RTT in order to get rid of possible outliers.
	// This value must be smaller than half TIME_REQUEST_RTT_BUFFER_SIZE
	const uint32 NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE = 1;
	// Number of RTT to calculate an average RTT for adjusting Server's clock delta time
	const uint32 TIME_REQUEST_RTT_BUFFER_SIZE = 10;
	// In order to get an accurate clock sync within the first ticks, the client will send a burst of time requests to
	// calculate a Server's clock delta time rapidly. Note: In this case, the '+ 1' is in case one of the burst messages
	// get lost
	const uint32 NUMBER_OF_INITIAL_TIME_REQUESTS_BURST = TIME_REQUEST_RTT_BUFFER_SIZE + 1;
	// By default, how often the client will send a time request message to adjust Server's clock delta time
	static const float32 DEFAULT_TIME_REQUESTS_FREQUENCY_SECONDS = 1.0f;

	class TimeSyncer
	{
		public:
			TimeSyncer();

			// void SetRequestFrequency( float32 frequency_seconds );

			/// <summary>
			/// <p>Updates the time sync requester. If a new time sync is needed it will be created and sent to
			/// the remote_peer.</p>
			///
			/// <p>Notes: A time request can happen in two cases: 1) If the initial burst is active and 2) If the time
			/// since the last time request is greater than the request frequency.</p>
			/// </summary>
			void Update( float32 elapsed_time, RemotePeer& remote_peer, MessageFactory& message_factory );
			void ProcessTimeResponse( const TimeResponseMessage& message );

		private:
			bool IsInitialBurstActive() const;
			bool IsSyncNeeded() const;

			float32 _requestFrequencySeconds;
			float32 _timeSinceLastTimeRequest;
			std::list< uint32 > _timeRequestRTTs;
			uint32 _numberOfInitialTimeRequestBurstLeft;
	};
} // namespace NetLib