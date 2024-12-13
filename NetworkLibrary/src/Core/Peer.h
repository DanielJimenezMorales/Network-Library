#pragma once
#include "NumericTypes.h"
#include <vector>
#include <list>

#include "Address.h"
#include "Socket.h"
#include "transmission_channels/TransmissionChannel.h"
#include "Delegate.h"
#include "RemotePeersHandler.h"
#include "NetworkEntityFactoryRegistry.h"

class Buffer;

namespace NetLib
{
	class Message;
	class NetworkPacket;
	class RemotePeer;
	class Buffer;
	class NetworkEntityFactory;

	enum ConnectionFailedReasonType : uint8
	{
		CFR_UNKNOWN = 0,			//Unexpect
		CFR_TIMEOUT = 1,			//The peer is inactive
		CFR_SERVER_FULL = 2,		//The server can't handle more connections, it has reached its maximum
		CFR_PEER_SHUT_DOWN = 3,		//The peer has shut down its Network system
		CFR_CONNECTION_TIMEOUT = 4	//The in process connection has taken too long
	};

	struct RemotePeerDisconnectionData
	{
		uint32 id;
		bool shouldNotify;
		ConnectionFailedReasonType reason;
	};

	enum PeerType : uint8
	{
		None = 0,
		ClientMode = 1,
		ServerMode = 2
	};

	enum PeerConnectionState : uint8
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
		bool Tick(float32 elapsedTime);
		bool Stop();

		PeerConnectionState GetConnectionState() const { return _connectionState; }
		PeerType GetPeerType() const { return _type; }
		void RegisterNetworkEntityFactory(INetworkEntityFactory* entityFactory);

		//Delegates related
		template<typename Functor>
		uint32 SubscribeToOnLocalPeerConnect(Functor&& functor);
		void UnsubscribeToOnPeerConnected(uint32 id);
		template<typename Functor>
		uint32 SubscribeToOnLocalPeerDisconnect(Functor&& functor);
		void UnsubscribeToOnPeerDisconnected(uint32 id);
		template<typename Functor>
		uint32 SubscribeToOnRemotePeerDisconnect(Functor&& functor);
		void UnsubscribeToOnRemotePeerDisconnect(uint32 id);
		template<typename Functor>
		uint32 SubscribeToOnRemotePeerConnect(Functor&& functor);
		void UnsubscribeToOnRemotePeerConnect(uint32 id);

		virtual ~Peer();

	protected:
		Peer(PeerType type, uint32 maxConnections, uint32 receiveBufferSize, uint32 sendBufferSize);
		Peer(const Peer&) = delete;

		Peer& operator=(const Peer&) = delete;

		virtual bool StartConcrete() = 0;
		virtual void ProcessMessageFromPeer(const Message& message, RemotePeer& remotePeer) = 0;
		virtual void ProcessMessageFromUnknownPeer(const Message& message, const Address& address) = 0;
		virtual void TickConcrete(float32 elapsedTime) = 0;
		virtual bool StopConcrete() = 0;

		void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;
		bool AddRemotePeer(const Address& addressInfo, uint16 id, uint64 clientSalt, uint64 serverSalt);
		void ConnectRemotePeer(RemotePeer& remotePeer);
		bool BindSocket(const Address& address) const;

		void StartDisconnectingRemotePeer(uint32 id, bool shouldNotify, ConnectionFailedReasonType reason);

		void RequestStop(bool shouldNotifyRemotePeers, ConnectionFailedReasonType reason);

		//Delegates related
		void ExecuteOnLocalPeerConnect();
		void ExecuteOnLocalPeerDisconnect(ConnectionFailedReasonType reason);

		RemotePeersHandler _remotePeersHandler;

		NetworkEntityFactoryRegistry _networkEntityFactoryRegistry;

	private:
		void ProcessReceivedData();
		void ProcessDatagram(Buffer& buffer, const Address& address);
		void ProcessNewRemotePeerMessages();

		void SetConnectionState(PeerConnectionState state);

		//Remote peer related
		void TickRemotePeers(float32 elapsedTime);
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

		bool DoesRemotePeerIdExistInPendingDisconnections(uint32 id) const;
		void FinishRemotePeersDisconnection();

		void StopInternal();

		//Delegates related
		void ExecuteOnRemotePeerConnect(uint32 remotePeerId);
		void ExecuteOnRemotePeerDisconnect();

		PeerType _type;
		PeerConnectionState _connectionState;
		Address _address;
		Socket _socket;

		uint32 _receiveBufferSize;
		uint8* _receiveBuffer;
		uint32 _sendBufferSize;
		uint8* _sendBuffer;

		//Stop request
		bool _isStopRequested;
		bool _stopRequestShouldNotifyRemotePeers;
		ConnectionFailedReasonType _stopRequestReason;

		std::list<RemotePeerDisconnectionData> _remotePeerPendingDisconnections;

		Common::Delegate<> _onLocalPeerConnect;
		Common::Delegate<ConnectionFailedReasonType> _onLocalPeerDisconnect;
		Common::Delegate<uint32> _onRemotePeerConnect;
		Common::Delegate<> _onRemotePeerDisconnect;
	};


	template<typename Functor>
	inline uint32 Peer::SubscribeToOnLocalPeerConnect(Functor&& functor)
	{
		return _onLocalPeerConnect.AddSubscriber(std::forward<Functor>(functor));
	}

	template<typename Functor>
	inline uint32 Peer::SubscribeToOnLocalPeerDisconnect(Functor&& functor)
	{
		return _onLocalPeerDisconnect.AddSubscriber(std::forward<Functor>(functor));
	}

	template<typename Functor>
	inline uint32 Peer::SubscribeToOnRemotePeerDisconnect(Functor&& functor)
	{
		return _onRemotePeerDisconnect.AddSubscriber(std::forward<Functor>(functor));
	}
	template<typename Functor>
	inline uint32 Peer::SubscribeToOnRemotePeerConnect(Functor&& functor)
	{
		return _onRemotePeerConnect.AddSubscriber(std::forward<Functor>(functor));
	}
}
