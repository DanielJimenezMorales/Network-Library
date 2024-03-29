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

	enum PeerType : uint8_t
	{
		None = 0,
		ClientMode = 1,
		ServerMode = 2
	};

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

		void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;
		bool AddRemoteClient(const Address& addressInfo, uint16_t id, uint64_t dataPrefix);
		int FindFreeRemotePeerSlot() const;
		RemotePeer* GetRemotePeerFromAddress(const Address& address);
		bool IsRemotePeerAlreadyConnected(const Address& address) const;
		PendingConnection* GetPendingConnectionFromAddress(const Address& address);
		bool IsPendingConnectionAlreadyAdded(const Address& address) const;
		void RemovePendingConnection(const Address& address);
		bool BindSocket(const Address& address) const;

		//Delegates related
		void ExecuteOnPeerConnected();
		void ExecuteOnPeerDisconnected();

		std::queue<unsigned int> _remotePeerSlotIDsToDisconnect;
		std::vector<PendingConnection> _pendingConnections;
		const int _maxConnections;
		std::vector<bool> _remotePeerSlots;
		std::vector<RemotePeer> _remotePeers;

	private:
		void ProcessReceivedData();
		void ProcessDatagram(Buffer& buffer, const Address& address);
		void ProcessNewRemotePeerMessages();

		void TickRemotePeers(float elapsedTime);
		void HandlerRemotePeersInactivity();

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

		PeerType _type;
		Address _address;
		Socket _socket;

		unsigned int _receiveBufferSize;
		uint8_t* _receiveBuffer;
		unsigned int _sendBufferSize;
		uint8_t* _sendBuffer;

		Common::Delegate<> _onPeerConnected;
		Common::Delegate<> _onPeerDisconnected;
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
}
