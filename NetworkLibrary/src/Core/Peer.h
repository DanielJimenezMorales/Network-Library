#pragma once
#include <cstdint>
#include <vector>
#include <list>

#include "Address.h"
#include "Socket.h"
#include "TransmissionChannel.h"
#include "Delegate.h"
#include "RemotePeersHandler.h"
#include "ReplicationManager.h"

class Buffer;

namespace NetLib
{
	class Message;
	class NetworkPacket;
	class RemotePeer;
	class NetworkEntityFactory;

	enum ConnectionFailedReasonType : uint8_t
	{
		CFR_UNKNOWN = 0,			//Unexpect
		CFR_TIMEOUT = 1,			//The peer is inactive
		CFR_SERVER_FULL = 2,		//The server can't handle more connections, it has reached its maximum
		CFR_PEER_SHUT_DOWN = 3,		//The peer has shut down its Network system
		CFR_CONNECTION_TIMEOUT = 4	//The in process connection has taken too long
	};

	struct RemotePeerDisconnectionData
	{
		unsigned int id;
		bool shouldNotify;
		ConnectionFailedReasonType reason;
	};

	enum PeerType : uint8_t
	{
		None = 0,
		ClientMode = 1,
		ServerMode = 2
	};

	enum PeerConnectionState : uint8_t
	{
		PCS_Disconnected = 0,
		PCS_Connecting = 1,
		PCS_Connected = 2
	};

	//TODO Set ordered and reliable flags in all the connection messages such as challenge response, connection approved...
	class Peer
	{
	public:
		bool Start();
		bool PreTick();
		bool Tick(float elapsedTime);
		bool Stop();

		PeerConnectionState GetConnectionState() const { return _connectionState; }
		PeerType GetPeerType() const { return _type; }
		void RegisterNetworkEntityFactory(INetworkEntityFactory* entityFactory);

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
		Peer(PeerType type, unsigned int maxConnections, unsigned int receiveBufferSize, unsigned int sendBufferSize);
		Peer(const Peer&) = delete;

		Peer& operator=(const Peer&) = delete;

		virtual bool StartConcrete() = 0;
		virtual void ProcessMessageFromPeer(const Message& message, RemotePeer& remotePeer) = 0;
		virtual void ProcessMessageFromUnknownPeer(const Message& message, const Address& address) = 0;
		virtual void TickConcrete(float elapsedTime) = 0;
		virtual bool StopConcrete() = 0;

		void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;
		bool AddRemotePeer(const Address& addressInfo, uint16_t id, uint64_t clientSalt, uint64_t serverSalt);
		void ConnectRemotePeer(RemotePeer& remotePeer);
		bool BindSocket(const Address& address) const;

		void StartDisconnectingRemotePeer(unsigned int id, bool shouldNotify, ConnectionFailedReasonType reason);

		void RequestStop(bool shouldNotifyRemotePeers, ConnectionFailedReasonType reason);

		//Delegates related
		void ExecuteOnLocalPeerConnect();
		void ExecuteOnLocalPeerDisconnect(ConnectionFailedReasonType reason);

		RemotePeersHandler _remotePeersHandler;
		ReplicationManager _replicationManager;

	private:
		void ProcessReceivedData();
		void ProcessDatagram(Buffer& buffer, const Address& address);
		void ProcessNewRemotePeerMessages();

		void SetConnectionState(PeerConnectionState state);

		//Remote peer related
		void TickRemotePeers(float elapsedTime);
		void DisconnectAllRemotePeers(bool shouldNotify, ConnectionFailedReasonType reason);
		void DisconnectRemotePeer(const RemotePeer& remotePeer, bool shouldNotify, ConnectionFailedReasonType reason);

		void CreateDisconnectionPacket(const RemotePeer& remotePeer, ConnectionFailedReasonType reason);

		void SendData();
		/// <summary>
		/// Sends pending data to all the connected remote peers
		/// </summary>
		void SendDataToRemotePeers();
		void SendDataToRemotePeer(RemotePeer& remotePeer);
		void SendPacketToRemotePeer(RemotePeer& remotePeer, TransmissionChannelType type);

		void SendDataToAddress(const Buffer& buffer, const Address& address) const;

		bool DoesRemotePeerIdExistInPendingDisconnections(unsigned int id) const;
		void FinishRemotePeersDisconnection();

		void StopInternal();

		//Delegates related
		void ExecuteOnRemotePeerConnect(uint32_t remotePeerId);
		void ExecuteOnRemotePeerDisconnect();

		PeerType _type;
		PeerConnectionState _connectionState;
		Address _address;
		Socket _socket;

		unsigned int _receiveBufferSize;
		uint8_t* _receiveBuffer;
		unsigned int _sendBufferSize;
		uint8_t* _sendBuffer;

		//Stop request
		bool _isStopRequested;
		bool _stopRequestShouldNotifyRemotePeers;
		ConnectionFailedReasonType _stopRequestReason;

		std::list<RemotePeerDisconnectionData> _remotePeerPendingDisconnections;

		Common::Delegate<> _onLocalPeerConnect;
		Common::Delegate<ConnectionFailedReasonType> _onLocalPeerDisconnect;
		Common::Delegate<uint32_t> _onRemotePeerConnect;
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
