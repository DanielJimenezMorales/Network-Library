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

	//TODO Create an enum for disconnection reasons (dont mix it with connection failed reasons)
	//TODO Create disconnection struct with remote peer index, a bool to say if it should notify peer with a disconnection message and an optional reason
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
		template<typename Functor>
		unsigned int SubscribeToOnRemotePeerDisconnect(Functor&& functor);
		void UnsubscribeToOnRemotePeerDisconnect(unsigned int id);

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
		bool RemovePendingConnectionAtIndex(unsigned int index);
		bool RemovePendingConnection(const Address& address);
		PendingConnection* GetPendingConnectionFromAddress(const Address& address);
		PendingConnection* GetPendingConnectionFromIndex(unsigned int index);

		void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;
		bool AddRemotePeer(const Address& addressInfo, uint16_t id, uint64_t dataPrefix);
		int FindFreeRemotePeerSlot() const;
		RemotePeer* GetRemotePeerFromAddress(const Address& address);
		int GetRemotePeerIndex(const RemotePeer& remotePeer) const;
		bool IsRemotePeerAlreadyConnected(const Address& address) const;
		bool BindSocket(const Address& address) const;

		void RemoveAllRemotePeers(ConnectionFailedReasonType reason);
		void RemoveRemotePeer(unsigned int remotePeerIndex, ConnectionFailedReasonType reason);
		void CreateDisconnectionPacket(const RemotePeer& remotePeer, ConnectionFailedReasonType reason);

		void StartDisconnectingRemotePeer(unsigned int index);

		//Delegates related
		template<typename Functor>
		unsigned int SubscribeToOnPendingConnectionTimedOut(Functor&& functor);
		void UnsubscribeToOnPendingConnectionTimedOut(unsigned int id);

		void ExecuteOnPeerConnected();
		void ExecuteOnPeerDisconnected();
		void ExecuteOnLocalConnectionFailed(ConnectionFailedReasonType reason);

		std::vector<bool> _pendingConnectionSlots;
		std::vector<PendingConnection> _pendingConnections;

		const int _maxConnections;
		std::vector<bool> _remotePeerSlots;
		std::vector<RemotePeer> _remotePeers;
		std::queue<unsigned int> _remotePeerSlotIDsToDisconnect;

	private:
		void ProcessReceivedData();
		void ProcessDatagram(Buffer& buffer, const Address& address);
		void ProcessNewRemotePeerMessages();

		//Pending connection related
		void TickPendingConnections(float elapsedTime);
		int GetPendingConnectionIndexFromAddress(const Address& address) const;

		//Remote peer related
		void TickRemotePeers(float elapsedTime);
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

		void FinishRemotePeersDisconnection();

		void StopInternal(ConnectionFailedReasonType reason);
		void ResetPendingConnections();

		//Delegates related
		void ExecuteOnPendingConnectionTimedOut(const Address& address);
		void ExecuteOnRemotePeerDisconnect();

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
		Common::Delegate<> _onRemotePeerDisconnect;
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

	template<typename Functor>
	inline unsigned int Peer::SubscribeToOnRemotePeerDisconnect(Functor&& functor)
	{
		return _onRemotePeerDisconnect.AddSubscriber(std::forward<Functor>(functor));
	}
}
