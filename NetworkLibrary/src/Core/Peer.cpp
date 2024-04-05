#include <sstream>
#include <memory>
#include <cassert>

#include "Peer.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "Logger.h"
#include "Buffer.h"
#include "RemotePeer.h"
#include "MessageFactory.h"

namespace NetLib
{
	bool Peer::Start()
	{
		if (_socket.Start() != SocketResult::SOKT_SUCCESS)
		{
			Common::LOG_ERROR("Error while starting peer, aborting operation...");
			return false;
		}

		if (!StartConcrete())
		{
			Common::LOG_ERROR("Error while starting peer, aborting operation...");
			return false;
		}

		MessageFactory::CreateInstance(3);

		return true;
	}

	bool Peer::Tick(float elapsedTime)
	{
		ProcessReceivedData();

		TickPendingConnections(elapsedTime);
		TickRemotePeers(elapsedTime);
		TickConcrete(elapsedTime);
		FinishRemotePeersDisconnection();

		SendData();

		return true;
	}

	bool Peer::Stop()
	{
		StopInternal(ConnectionFailedReasonType::CFR_PEER_SHUT_DOWN);
		ExecuteOnPeerDisconnected();

		return true;
	}

	void Peer::UnsubscribeToOnRemotePeerDisconnect(unsigned int id)
	{
		_onRemotePeerDisconnect.DeleteSubscriber(id);
	}

	void Peer::UnsubscribeToOnRemotePeerConnect(unsigned int id)
	{
		_onRemotePeerConnect.DeleteSubscriber(id);
	}

	Peer::~Peer()
	{
		delete[] _receiveBuffer;
		delete[] _sendBuffer;
	}

	Peer::Peer(PeerType type, unsigned int maxConnections, unsigned int receiveBufferSize, unsigned int sendBufferSize) :
		_type(type),
		_socket(),
		_address(Address::GetInvalid()),
		_receiveBufferSize(receiveBufferSize),
		_sendBufferSize(sendBufferSize),
		_remotePeersHandler(maxConnections),
		_onLocalPeerConnect(),
		_onLocalPeerDisconnect()
	{
		_receiveBuffer = new uint8_t[_receiveBufferSize];
		_sendBuffer = new uint8_t[_sendBufferSize];

		const unsigned int maxPendingConnections = maxConnections * 2;
		_pendingConnectionSlots.reserve(maxPendingConnections);
		_pendingConnections.reserve(maxPendingConnections); //There could be more pending connections than clients

		for (size_t i = 0; i < maxPendingConnections; ++i)
		{
			_pendingConnectionSlots.push_back(false);
			_pendingConnections.emplace_back();
		}
	}

	void Peer::SendPacketToAddress(const NetworkPacket& packet, const Address& address) const
	{
		Buffer buffer = Buffer(_sendBuffer, packet.Size());
		packet.Write(buffer);

		_socket.SendTo(_sendBuffer, packet.Size(), address);
	}

	bool Peer::AddRemotePeer(const Address& addressInfo, uint16_t id, uint64_t dataPrefix)
	{
		bool addedSuccesfully = _remotePeersHandler.AddRemotePeer(addressInfo, id, dataPrefix);

		if (addedSuccesfully)
		{
			ExecuteOnRemotePeerConnect();
		}

		return addedSuccesfully;
	}

	PendingConnection* Peer::GetPendingConnectionFromAddress(const Address& address)
	{
		PendingConnection* result = nullptr;
		int id = GetPendingConnectionIndexFromAddress(address);
		if (id != -1)
		{
			result = &_pendingConnections[id];
		}

		return result;
	}

	PendingConnection* Peer::GetPendingConnectionFromIndex(unsigned int id)
	{
		PendingConnection* result = nullptr;
		if (_pendingConnectionSlots[id])
		{
			result = &_pendingConnections[id];
		}

		return result;
	}

	bool Peer::RemovePendingConnection(const Address& address)
	{
		int id = GetPendingConnectionIndexFromAddress(address);
		if (id != -1)
		{
			return RemovePendingConnectionAtIndex(id);
		}

		return false;
	}

	bool Peer::BindSocket(const Address& address) const
	{
		SocketResult result = _socket.Bind(address);
		if (result != SocketResult::SOKT_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void Peer::DisconnectAllRemotePeers(bool shouldNotify, ConnectionFailedReasonType reason)
	{
		if (shouldNotify)
		{
			auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
			auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

			for (; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt)
			{
				const RemotePeer& remotePeer = **validRemotePeersIt;
				CreateDisconnectionPacket(remotePeer, reason);
			}
		}

		_remotePeersHandler.RemoveAllRemotePeers();
	}

	void Peer::DisconnectRemotePeer(const RemotePeer& remotePeer, bool shouldNotify, ConnectionFailedReasonType reason)
	{
		if (shouldNotify)
		{
			CreateDisconnectionPacket(remotePeer, reason);
		}

		bool removedSuccesfully = _remotePeersHandler.RemoveRemotePeer(remotePeer.GetClientIndex());
		assert(removedSuccesfully);
		if (removedSuccesfully)
		{
			ExecuteOnRemotePeerDisconnect();
		}
	}

	void Peer::CreateDisconnectionPacket(const RemotePeer& remotePeer, ConnectionFailedReasonType reason)
	{
		NetworkPacket packet;
		packet.SetHeaderChannelType(TransmissionChannelType::UnreliableUnordered);

		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::Disconnection);

		std::unique_ptr<DisconnectionMessage> disconenctionMessage(static_cast<DisconnectionMessage*>(message.release()));
		disconenctionMessage->SetOrdered(false);
		disconenctionMessage->SetReliability(false);
		disconenctionMessage->prefix = remotePeer.GetDataPrefix();
		disconenctionMessage->reason = reason;

		packet.AddMessage(std::move(disconenctionMessage));
		SendPacketToAddress(packet, remotePeer.GetAddress());
	}

	int Peer::AddPendingConnection(const Address& addr, float timeoutSeconds)
	{
		int id = -1;
		for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
		{
			if (!_pendingConnectionSlots[i])
			{
				_pendingConnectionSlots[i] = true;
				_pendingConnections[i].Initialize(addr, timeoutSeconds);
				id = i;
				break;
			}
		}

		return id;
	}

	bool Peer::RemovePendingConnectionAtIndex(unsigned int id)
	{
		if (_pendingConnectionSlots[id])
		{
			_pendingConnectionSlots[id] = false;
			_pendingConnections[id].Reset();

			Common::LOG_INFO("REMOVING PENDING CONNECTION");
			return true;
		}

		return false;
	}

	void Peer::ExecuteOnPeerConnected()
	{
		_onLocalPeerConnect.Execute();
	}

	void Peer::ExecuteOnPeerDisconnected()
	{
		_onLocalPeerDisconnect.Execute();
	}

	void Peer::ExecuteOnLocalConnectionFailed(ConnectionFailedReasonType reason)
	{
		_onLocalConnectionFailed.Execute(reason);
	}

	void Peer::UnsubscribeToOnPeerConnected(unsigned int id)
	{
		_onLocalPeerConnect.DeleteSubscriber(id);
	}

	void Peer::UnsubscribeToOnPeerDisconnected(unsigned int id)
	{
		_onLocalPeerDisconnect.DeleteSubscriber(id);
	}

	void Peer::UnsubscribeToOnPendingConnectionTimedOut(unsigned int id)
	{
		_onPendingConnectionTimedOut.DeleteSubscriber(id);
	}

	void Peer::ProcessReceivedData()
	{
		Address remoteAddress = Address::GetInvalid();
		unsigned int numberOfBytesRead = 0;
		bool arePendingDatagramsToRead = true;

		do
		{
			SocketResult result = _socket.ReceiveFrom(_receiveBuffer, _receiveBufferSize, &remoteAddress, numberOfBytesRead);

			if (result == SocketResult::SOKT_SUCCESS)
			{
				//Data read succesfully. Keep going!
				Buffer buffer = Buffer(_receiveBuffer, numberOfBytesRead);
				ProcessDatagram(buffer, remoteAddress);
			}
			else if (result == SocketResult::SOKT_ERR || result == SocketResult::SOKT_WOULDBLOCK)
			{
				//An unexpected error occurred or there is no more data to read atm
				arePendingDatagramsToRead = false;
			}
			else if (result == SocketResult::SOKT_CONNRESET)
			{
				//The remote socket got closed unexpectedly
				RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(remoteAddress);
				if (remotePeer != nullptr)
				{
					StartDisconnectingRemotePeer(remotePeer->GetClientIndex(), false, ConnectionFailedReasonType::CFR_UNKNOWN);
				}
			}
		} while (arePendingDatagramsToRead);

		ProcessNewRemotePeerMessages();
	}

	void Peer::ProcessDatagram(Buffer& buffer, const Address& address)
	{
		//Read incoming packet
		NetworkPacket packet = NetworkPacket();
		packet.Read(buffer);

		RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);
		bool isPacketFromRemotePeer = (remotePeer != nullptr);

		//Process packet ACKs
		if (isPacketFromRemotePeer)
		{
			uint32_t acks = packet.GetHeader().ackBits;
			uint16_t lastAckedMessageSequenceNumber = packet.GetHeader().lastAckedSequenceNumber;
			TransmissionChannelType channelType = static_cast<TransmissionChannelType>(packet.GetHeader().channelType);
			remotePeer->ProcessACKs(acks, lastAckedMessageSequenceNumber, channelType);
		}

		//Process packet messages one by one
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		while (packet.GetNumberOfMessages() > 0)
		{
			std::unique_ptr<Message> message = packet.GetMessages();
			if (isPacketFromRemotePeer)
			{
				remotePeer->AddReceivedMessage(std::move(message));
			}
			else
			{
				ProcessMessage(*message, address);

				messageFactory.ReleaseMessage(std::move(message));
			}
		}
	}

	void Peer::ProcessNewRemotePeerMessages()
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for (; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt)
		{
			//Process ready to process messages from remote peer
			RemotePeer& remotePeer = **validRemotePeersIt;

			while (remotePeer.ArePendingReadyToProcessMessages())
			{
				const Message* message = remotePeer.GetPendingReadyToProcessMessage();

				ProcessMessage(*message, remotePeer.GetAddress());
			}

			remotePeer.FreeProcessedMessages();
		}
	}

	void Peer::TickPendingConnections(float elapsedTime)
	{
		unsigned int numberOfPendingConnections = _pendingConnections.size();

		std::vector<unsigned int> inactivePendingConnectionIndexes;
		inactivePendingConnectionIndexes.reserve(numberOfPendingConnections);

		//Update pending connections
		for (unsigned int i = 0; i < numberOfPendingConnections; ++i)
		{
			if (_pendingConnectionSlots[i])
			{
				_pendingConnections[i].Tick(elapsedTime);
				if (_pendingConnections[i].IsInactive())
				{
					inactivePendingConnectionIndexes.push_back(i);
				}
			}
		}

		//Delete pending connections that have timed out (That means they are inactive!)
		int inactiveIndex = -1;
		Address inactiveAddress = Address::GetInvalid();
		for (size_t i = 0; i < inactivePendingConnectionIndexes.size(); ++i)
		{
			inactiveIndex = inactivePendingConnectionIndexes[i];
			inactiveAddress = _pendingConnections[inactiveIndex].GetAddress();
			RemovePendingConnectionAtIndex(inactiveIndex);

			ExecuteOnPendingConnectionTimedOut(inactiveAddress);
		}
	}

	void Peer::TickRemotePeers(float elapsedTime)
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for (; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt)
		{
			RemotePeer& remotePeer = **validRemotePeersIt;
			remotePeer.Tick(elapsedTime);

			//Start the disconnection process for those ones who are inactive
			if (remotePeer.IsInactive())
			{
				StartDisconnectingRemotePeer(remotePeer.GetClientIndex(), true, ConnectionFailedReasonType::CFR_TIMEOUT);
			}
		}
	}

	int Peer::GetPendingConnectionIndexFromAddress(const Address& address) const
	{
		int id = -1;
		for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
		{
			if (_pendingConnectionSlots[i])
			{
				if (_pendingConnections[i].GetAddress() == address)
				{
					id = i;
					break;
				}
			}
		}

		return id;
	}

	void Peer::SendData()
	{
		SendDataToPendingConnections();
		SendDataToRemotePeers();
	}

	void Peer::SendDataToPendingConnections()
	{
		for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
		{
			if (_pendingConnectionSlots[i])
			{
				SendDataToPendingConnection(_pendingConnections[i]);
			}
		}
	}

	void Peer::SendDataToPendingConnection(PendingConnection& pendingConnection)
	{
		if (!pendingConnection.ArePendingMessages())
		{
			return;
		}

		//Create and populate packet
		NetworkPacket packet = NetworkPacket();
		do
		{
			std::unique_ptr<Message> message = pendingConnection.GetAMessage();
			packet.AddMessage(std::move(message));
		} while (pendingConnection.ArePendingMessages());

		//Send packet
		SendPacketToAddress(packet, pendingConnection.GetAddress());

		//Send messages ownership back to pending connection
		while (packet.GetNumberOfMessages() > 0)
		{
			std::unique_ptr<Message> message = packet.GetMessages();
			pendingConnection.AddSentMessage(std::move(message));
		}

		//Release sent messages
		pendingConnection.FreeSentMessages();
	}

	void Peer::SendDataToRemotePeers()
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for (; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt)
		{
			SendDataToRemotePeer(**validRemotePeersIt);
		}
	}

	void Peer::SendDataToRemotePeer(RemotePeer& remotePeer)
	{
		//Send one packet per Remote peer transmission channel
		std::vector<TransmissionChannelType> channelTypes = remotePeer.GetAvailableTransmissionChannelTypes();
		std::vector<TransmissionChannelType>::const_iterator cit = channelTypes.cbegin();
		for (cit; cit != channelTypes.cend(); ++cit)
		{
			TransmissionChannelType channelType = *cit;
			SendPacketToRemotePeer(remotePeer, channelType);
		}

		remotePeer.FreeSentMessages();
	}

	void Peer::SendPacketToRemotePeer(RemotePeer& remotePeer, TransmissionChannelType type)
	{
		if (!remotePeer.ArePendingMessages(type) && !remotePeer.AreUnsentACKs(type))
		{
			return;
		}

		NetworkPacket packet = NetworkPacket();

		//TODO Check somewhere if there is a message larger than the maximum packet size. Log a warning saying that the message will never get sent and delete it.
		//TODO Include data prefix in packet's header and check if the data prefix is correct when receiving a packet

		//Check if we should include a message to the packet
		bool arePendingMessages = remotePeer.ArePendingMessages(type);
		bool isThereCapacityLeft = packet.CanMessageFit(remotePeer.GetSizeOfNextUnsentMessage(type));

		while (arePendingMessages && isThereCapacityLeft)
		{
			//Configure and add message to packet
			std::unique_ptr<Message> message = remotePeer.GetPendingMessage(type);

			if (message->GetHeader().isReliable)
			{
				std::stringstream ss;
				ss << "Reliable message sequence number: " << message->GetHeader().messageSequenceNumber << " Message type: " << (int)message->GetHeader().type;
				Common::LOG_INFO(ss.str());
			}

			packet.AddMessage(std::move(message));

			//Check if we should include another message to the packet
			arePendingMessages = remotePeer.ArePendingMessages(type);
			isThereCapacityLeft = packet.CanMessageFit(remotePeer.GetSizeOfNextUnsentMessage(type));
		}

		//Set packet header fields
		uint32_t acks = remotePeer.GenerateACKs(type);
		packet.SetHeaderACKs(acks);

		uint16_t lastAckedMessageSequenceNumber = remotePeer.GetLastMessageSequenceNumberAcked(type);
		packet.SetHeaderLastAcked(lastAckedMessageSequenceNumber);

		packet.SetHeaderChannelType(type);

		SendPacketToAddress(packet, remotePeer.GetAddress());
		remotePeer.SeUnsentACKsToFalse(type);

		//Send messages ownership back to remote peer
		while (packet.GetNumberOfMessages() > 0)
		{
			std::unique_ptr<Message> message = packet.GetMessages();
			remotePeer.AddSentMessage(std::move(message), type);
		}
	}

	void Peer::SendDataToAddress(const Buffer& buffer, const Address& address) const
	{
		_socket.SendTo(buffer.GetData(), buffer.GetSize(), address);
	}

	void Peer::StartDisconnectingRemotePeer(unsigned int id, bool shouldNotify, ConnectionFailedReasonType reason)
	{
		RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromId(id);

		if (remotePeer != nullptr)
		{
			if (!DoesRemotePeerIdExistInPendingDisconnections(id))
			{
				Common::LOG_INFO("EMPIEZO A DESCONECTARR");
				RemotePeerDisconnectionData disconnectionData;
				disconnectionData.id = id;
				disconnectionData.shouldNotify = shouldNotify;
				disconnectionData.reason = reason;

				_remotePeerPendingDisconnections.push_back(disconnectionData);
			}
		}
	}

	bool Peer::DoesRemotePeerIdExistInPendingDisconnections(unsigned int id) const
	{
		bool doesIdAlreadyExist = false;
		auto cit = _remotePeerPendingDisconnections.cbegin();
		while (cit != _remotePeerPendingDisconnections.cend())
		{
			const RemotePeerDisconnectionData& data = *cit;
			if (data.id == id)
			{
				doesIdAlreadyExist = true;
				break;
			}

			++cit;
		}

		return doesIdAlreadyExist;
	}

	void Peer::FinishRemotePeersDisconnection()
	{
		while (!_remotePeerPendingDisconnections.empty())
		{
			RemotePeerDisconnectionData& disconnectionData = _remotePeerPendingDisconnections.front();

			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromId(disconnectionData.id);
			if (remotePeer != nullptr)
			{
				DisconnectRemotePeer(*remotePeer, disconnectionData.shouldNotify, disconnectionData.reason);
			}

			_remotePeerPendingDisconnections.erase(_remotePeerPendingDisconnections.begin());
		}
	}

	void Peer::ExecuteOnPendingConnectionTimedOut(const Address& address)
	{
		_onPendingConnectionTimedOut.Execute(address);
	}

	void Peer::ExecuteOnRemotePeerDisconnect()
	{
		_onRemotePeerDisconnect.Execute();
	}

	void Peer::ExecuteOnRemotePeerConnect()
	{
		_onRemotePeerConnect.Execute();
	}

	void Peer::StopInternal(ConnectionFailedReasonType reason)
	{
		StopConcrete();

		ResetPendingConnections();
		DisconnectAllRemotePeers(true, reason);

		_socket.Close();

		MessageFactory::DeleteInstance();
	}

	void Peer::ResetPendingConnections()
	{
		for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
		{
			if (_pendingConnectionSlots[i])
			{
				RemovePendingConnectionAtIndex(i);
			}
		}
	}
}
