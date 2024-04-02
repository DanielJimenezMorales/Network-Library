#pragma once
#include <cstdint>
#include <vector>
#include <queue>

#include "Address.h"
#include "PendingConnection.h"
#include "Socket.h"
#include "Buffer.h"
#include "TransmissionChannel.h"
#include "Delegate.h"

namespace NetLib
{
	class Message;
	class NetworkPacket;
	class RemotePeer;

#define REMOTE_CLIENT_INACTIVITY_TIME 5.0f

	enum ConnectionFailedReasonType : uint8_t
	{
		CFR_UNKNOWN = 0,
		CFR_TIMEOUT = 1,
		CFR_SERVER_FULL = 2,
		CFR_PEER_SHUT_DOWN = 3
	};

	enum PeerType : uint8_t
	{
		None = 0,
		ClientMode = 1,
		ServerMode = 2
	};

	//TODO Think how to handle connection timed out on client and disconnection on both client and server in order to make it work correctly
	class Peer
	{
	public:
		bool Start();
		bool Tick(float elapsedTime);
		bool Stop();

		//Delegates related
		template<typename Functor>
		unsigned int SubscribeToOnPeerConnected(Functor&& functor);
		void UnsubscribeToOnPeerConnected(unsigned int id);
		template<typename Functor>
		unsigned int SubscribeToOnPeerDisconnected(Functor&& functor);
		void UnsubscribeToOnPeerDisconnected(unsigned int id);

		virtual ~Peer();

	protected:
		Peer(PeerType type, int maxConnections, unsigned int receiveBufferSize, unsigned int sendBufferSize);
		Peer(const Peer&) = delete;

		Peer& operator=(const Peer&) = delete;

		virtual bool StartConcrete() = 0;
		virtual void ProcessMessage(const Message& message, const Address& address) = 0;
		virtual void TickConcrete(float elapsedTime) = 0;
		virtual void DisconnectRemotePeerConcrete(RemotePeer& remotePeer) = 0;
		virtual bool StopConcrete() = 0;

		//Pending connection related
		int AddPendingConnection(const Address& addr, float timeoutSeconds);
		bool DeletePendingConnectionAtIndex(unsigned int index);
		PendingConnection* GetPendingConnectionFromAddress(const Address& address);
		PendingConnection* GetPendingConnectionFromIndex(unsigned int index);

		void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;
		bool AddRemotePeer(const Address& addressInfo, uint16_t id, uint64_t dataPrefix);
		int FindFreeRemotePeerSlot() const;
		RemotePeer* GetRemotePeerFromAddress(const Address& address);
		bool IsRemotePeerAlreadyConnected(const Address& address) const;
		bool IsPendingConnectionAlreadyAdded(const Address& address) const;//TODO Delete this one, it is not being used
		void RemovePendingConnection(const Address& address);//TODO Rewrite this method
		bool BindSocket(const Address& address) const;

		void RemoveAllRemotePeers(ConnectionFailedReasonType reason);
		void RemoveRemotePeer(unsigned int remotePeerIndex, ConnectionFailedReasonType reason);
		void CreateDisconnectionPacket(const RemotePeer& remotePeer, ConnectionFailedReasonType reason);

		//Delegates related
		template<typename Functor>
		unsigned int SubscribeToOnPendingConnectionTimedOut(Functor&& functor);
		void UnsubscribeToOnPendingConnectionTimedOut(unsigned int id);

		void ExecuteOnPeerConnected();
		void ExecuteOnPeerDisconnected();
		void ExecuteOnLocalConnectionFailed(ConnectionFailedReasonType reason);

		std::queue<unsigned int> _remotePeerSlotIDsToDisconnect;
		//TODO Create slots for handle fixed-size number of maximum connections
		std::vector<bool> _pendingConnectionSlots;
		std::vector<PendingConnection> _pendingConnections;
		const int _maxConnections;
		std::vector<bool> _remotePeerSlots;
		std::vector<RemotePeer> _remotePeers;

	private:
		void ProcessReceivedData();
		void ProcessDatagram(Buffer& buffer, const Address& address);
		void ProcessNewRemotePeerMessages();

		void TickPendingConnections(float elapsedTime);
		void TickRemotePeers(float elapsedTime);

		int GetPendingConnectionIndexFromAddress(const Address& address) const;
		int GetRemotePeerIndexFromAddress(const Address& address) const;

		void SendData();
		/// <summary>
		/// Sends pending data to all the active pending connections
		/// </summary>
		void SendDataToPendingConnections();
		void SendDataToPendingConnection(PendingConnection& pendingConnection);
		/// <summary>
		/// Sends pending data to all the connected remote peers
		/// </summary>
		void SendDataToRemotePeers();
		void SendDataToRemotePeer(RemotePeer& remotePeer);
		void SendPacketToRemotePeer(RemotePeer& remotePeer, TransmissionChannelType type);

		void SendDataToAddress(const Buffer& buffer, const Address& address) const;

		void StartDisconnectingRemotePeer(unsigned int index);
		void FinishRemotePeersDisconnection();

		void StopInternal(ConnectionFailedReasonType reason);
		void ResetPendingConnections();

		//Delegates related
		void ExecuteOnPendingConnectionTimedOut(const Address& address);

		PeerType _type;
		Address _address;
		Socket _socket;

		unsigned int _receiveBufferSize;
		uint8_t* _receiveBuffer;
		unsigned int _sendBufferSize;
		uint8_t* _sendBuffer;

		Common::Delegate<> _onPeerConnected;
		Common::Delegate<> _onPeerDisconnected;
		//This should only be called in client-side since the server is not connecting to anything.
		Common::Delegate<ConnectionFailedReasonType> _onLocalConnectionFailed;
		Common::Delegate<const Address&> _onPendingConnectionTimedOut;
	};


	template<typename Functor>
	inline unsigned int Peer::SubscribeToOnPeerConnected(Functor&& functor)
	{
		return _onPeerConnected.AddSubscriber(std::forward<Functor>(functor));
	}

	template<typename Functor>
	inline unsigned int Peer::SubscribeToOnPeerDisconnected(Functor&& functor)
	{
		return _onPeerDisconnected.AddSubscriber(std::forward<Functor>(functor));
	}

	template<typename Functor>
	inline unsigned int Peer::SubscribeToOnPendingConnectionTimedOut(Functor&& functor)
	{
		return _onPendingConnectionTimedOut.AddSubscriber(std::forward<Functor>(functor));
	}
}
