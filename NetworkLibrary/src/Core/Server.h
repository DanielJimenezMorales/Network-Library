#pragma once

#include <vector>

#include "Peer.h"
#include "RemotePeerInputsHandler.h"

namespace NetLib
{
	class PendingConnection;
	class ConnectionRequestMessage;
	class ConnectionChallengeResponseMessage;
	class TimeRequestMessage;
	class InGameMessage;
	class InputStateMessage;
	class DisconnectionMessage;
	class IInputState;
	class IInputStateFactory;

	class Server : public Peer
	{
	public:
		Server(int maxConnections);
		Server(const Server&) = delete;

		Server& operator=(const Server&) = delete;

		uint32_t CreateNetworkEntity(uint32_t entityType, uint32_t controlledByPeerId, float posX, float posY);
		void DestroyNetworkEntity(uint32_t entityId);
		//TODO Create a method for destroying all network entities controlled by a remote peer
		void RegisterInputStateFactory(IInputStateFactory* factory);
		const IInputState* GetInputFromRemotePeer(uint32_t remotePeerId);

		~Server() override;

	protected:
		bool StartConcrete() override;
		void ProcessMessageFromPeer(const Message& message, RemotePeer& remotePeer) override;
		void ProcessMessageFromUnknownPeer(const Message& message, const Address& address) override;
		void TickConcrete(float elapsedTime) override;
		bool StopConcrete() override;

	private:
		uint64_t GenerateServerSalt() const;

		void ProcessConnectionRequest(const ConnectionRequestMessage& message, const Address& address);
		void ProcessConnectionChallengeResponse(const ConnectionChallengeResponseMessage& message, RemotePeer& remotePeer);
		void ProcessTimeRequest(const TimeRequestMessage& message, RemotePeer& remotePeer);
		void ProcessInGame(const InGameMessage& message, RemotePeer& remotePeer);
		void ProcessInputs(const InputStateMessage& message, RemotePeer& remotePeer);
		void ProcessDisconnection(const DisconnectionMessage& message, RemotePeer& remotePeer);

		/// <summary>
		/// This method checks if a new client is able to connect to server
		/// </summary>
		/// <param name="address">The network address information of the client trying to connect</param>
		/// <returns>
		/// 0 = Is able to connect.
		/// 1 = Is already connected.
		/// -1 = Unable to connect, the server has reached its maximum connections.
		/// </returns>
		//int IsRemotePeerAbleToConnect(const Address& address) const;

		void CreateConnectionChallengeMessage(RemotePeer& remotePeer);
		void CreateConnectionApprovedMessage(RemotePeer& remotePeer);
		void CreateDisconnectionMessage(RemotePeer& remotePeer);
		void CreateTimeResponseMessage(RemotePeer& remotePeer, const TimeRequestMessage& timeRequest);
		void CreateInGameResponseMessage(RemotePeer& remotePeer, uint64_t data);
		void SendConnectionDeniedPacket(const Address& address, ConnectionFailedReasonType reason) const;
		void SendPacketToRemotePeer(const RemotePeer& remotePeer, const NetworkPacket& packet) const;

		void TickReplication();

		unsigned int _nextAssignedRemotePeerID = 1;

		RemotePeerInputsHandler _remotePeerInputsHandler;
		IInputStateFactory* _inputsFactory;
	};
}
