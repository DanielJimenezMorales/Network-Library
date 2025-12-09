#pragma once
#include "numeric_types.h"

#include "core/address.h"
#include "transmission_channels/unreliable_unordered_transmission_channel.h"
#include "metrics/metrics_handler.h"

namespace NetLib
{
	enum class ConnectingRemotePeerState : uint8
	{
		ConnectionRequest = 0,
		ConnectionChallenge = 1
	};

	class ConnectingRemotePeer
	{
		public:
			ConnectingRemotePeer();

			void StartUp();
			void ShutDown();

			void Connect( const Address& address, uint64 clientSalt, uint64 serverSalt );
			void Disconnect();

		private:
			Address _address;
			ConnectingRemotePeerState _currentState;

			float32 _maxInactivityTime;
			float32 _inactivityTimeLeft;
			uint64 _clientSalt;
			uint64 _serverSalt;

			UnreliableUnorderedTransmissionChannel _transmissionChannel;
	};
} // namespace NetLib
