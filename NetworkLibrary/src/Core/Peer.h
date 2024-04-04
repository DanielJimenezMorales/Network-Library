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

	enum ConnectionFailedReasonType : uint8_t
	{
		CFR_UNKNOWN = 0,			//Unexpect
		CFR_TIMEOUT = 1,			//The peer is inactive
		CFR_SERVER_FULL = 2,		//The server can't handle more connections, it has reached its maximum
		CFR_PEER_SHUT_DOWN = 3,		//The peer has shut down its Network system
		CFR_CONNECTION_TIMEOUT = 4	//The in process connection has taken too long
	};

	enum PeerType : uint8_t
	{
		None = 0,
		ClientMode = 1,
		ServerMode = 2
	};

	struct RemotePeerDisconnectionData
	{
		unsigned int index;
		bool shouldNotify;
		ConnectionFailedReasonType reason;
	};

	//If the local peer does not receive any message from a remote peer during REMOTE_PEER_INACTIVITY_TIME seconds it will be considered inactive and it will
	//be disconnected with ConnectionFailedReasonType::CFR_TIMEOUT reason
	const float REMOTE_PEER_INACTIVITY_TIME = 5.0f;

	//TODO Process pending connections as remote peers in order to be able to track inactivity
	//TODO Also check for pending connections on CONNRESET socket error when calling to receive from in order to remove pending connections too
	//Finish implementing OnConnection Failed on client when the connection has timed out
	class Peer
	{
	public:
		bool Start();
		bool Tick(float elapsedTime);
		bool Stop();

		//Delegates related
		template<typename Functor>
		unsigned int SubscribeToOnLocalPeerConnect(Functor&& functor);
		void UnsubscribeToOnPeerConnected(unsigned int id);
		template<typename Functor>
		unsigned int SubscribeToOnLocalPeerDisconnect(Functor&& functor);
		void UnsubscribeToOnPeerDisconnected(unsigned int id);
		template<typename Functor>
		unsigned int SubscribeToOnRemotePeerDisconnect(Functor&& functor);
		void UnsubscribeToOnRemotePeerDisconnect(unsigned int id);
		template<typename Functor>
		unsigned int SubscribeToOnRemotePeerConnect(Functor&& functor);
		void UnsubscribeToOnRemotePeerConnect(unsigned int id);

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

		void CreateDisconnectionPacket(const RemotePeer& remotePeer, ConnectionFailedReasonType reason);

		void StartDisconnectingRemotePeer(unsigned int index, bool shouldNotify, ConnectionFailedReasonType reason);

		void StopInternal(ConnectionFailedReasonType reason);

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
		void RemoveAllRemotePeers(bool shouldNotify, ConnectionFailedReasonType reason);
		void RemoveRemotePeer(unsigned int remotePeerIndex, bool shouldNotify, ConnectionFailedReasonType reason);

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

		void ResetPendingConnections();

		//Delegates related
		void ExecuteOnPendingConnectionTimedOut(const Address& address);
		void ExecuteOnRemotePeerDisconnect();
		void ExecuteOnRemotePeerConnect();

		PeerType _type;
		Address _address;
		Socket _socket;

		unsigned int _receiveBufferSize;
		uint8_t* _receiveBuffer;
		unsigned int _sendBufferSize;
		uint8_t* _sendBuffer;

		std::queue<RemotePeerDisconnectionData> _remotePeerDisconnections;

		Common::Delegate<> _onLocalPeerConnect;
		Common::Delegate<> _onLocalPeerDisconnect;
		//This should only be called in client-side since the server is not connecting to anything.
		Common::Delegate<ConnectionFailedReasonType> _onLocalConnectionFailed;
		Common::Delegate<const Address&> _onPendingConnectionTimedOut;
		Common::Delegate<> _onRemotePeerConnect;
		Common::Delegate<> _onRemotePeerDisconnect;
	};


	template<typename Functor>
	inline unsigned int Peer::SubscribeToOnLocalPeerConnect(Functor&& functor)
	{
		return _onLocalPeerConnect.AddSubscriber(std::forward<Functor>(functor));
	}

	template<typename Functor>
	inline unsigned int Peer::SubscribeToOnLocalPeerDisconnect(Functor&& functor)
	{
		return _onLocalPeerDisconnect.AddSubscriber(std::forward<Functor>(functor));
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
	template<typename Functor>
	inline unsigned int Peer::SubscribeToOnRemotePeerConnect(Functor&& functor)
	{
		return _onRemotePeerConnect.AddSubscriber(std::forward<Functor>(functor));
	}
}
