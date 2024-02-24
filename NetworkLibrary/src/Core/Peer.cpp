#include <sstream>

#include "Peer.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "Logger.h"
#include "Buffer.h"
#include "RemotePeer.h"
#include "MessageFactory.h"

bool Peer::Start()
{
	if (!InitializeSocketsLibrary())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}

	if (_socket.Start() != SocketResult::SUCCESS)
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}

	if (!StartConcrete())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}

	return true;
}

bool Peer::Tick(float elapsedTime)
{
	ProcessReceivedData();

	TickRemotePeers(elapsedTime);
	HandlerRemotePeersInactivity();
	TickConcrete(elapsedTime);

	SendData();

	FinishRemotePeersDisconnection();
	return true;
}

bool Peer::Stop()
{
	StopConcrete();
	_socket.Close();
	WSACleanup();

	return true;
}

Peer::~Peer()
{
	_remotePeerSlots.clear();
	_remotePeers.clear();

	delete[] _receiveBuffer;
	delete[] _sendBuffer;
}

Peer::Peer(PeerType type, int maxConnections, unsigned int receiveBufferSize, unsigned int sendBufferSize) :
		_type(type),
		_socket(),
		_address(Address::GetInvalid()),
		_maxConnections(maxConnections),
		_receiveBufferSize(receiveBufferSize),
		_sendBufferSize(sendBufferSize)
{
	_receiveBuffer = new uint8_t[_receiveBufferSize];
	_sendBuffer = new uint8_t[_sendBufferSize];

	if (_maxConnections > 0)
	{
		_remotePeerSlots.reserve(_maxConnections);
		_remotePeers.reserve(_maxConnections);
		_pendingConnections.reserve(_maxConnections * 2); //There could be more pending connections than clients

		for (size_t i = 0; i < _maxConnections; ++i)
		{
			_remotePeerSlots.push_back(false); //You could use vector.resize in these ones.
			_remotePeers.emplace_back();
		}
	}
}

void Peer::SendPacketToAddress(const NetworkPacket& packet, const Address& address) const
{
	Buffer buffer = Buffer(_sendBuffer, packet.Size());
	packet.Write(buffer);

	_socket.SendTo(_sendBuffer, packet.Size(), address);
}

bool Peer::AddRemoteClient(const Address& addressInfo, uint16_t id, uint64_t dataPrefix)
{
	int slotIndex = FindFreeRemoteClientSlot();
	if(slotIndex == -1)
	{
		return false;
	}
	
	_remotePeerSlots[slotIndex] = true;
	_remotePeers[slotIndex].Connect(addressInfo.GetInfo(), id, REMOTE_CLIENT_INACTIVITY_TIME, dataPrefix);
	return true;
}

int Peer::FindFreeRemoteClientSlot() const
{
	for (int i = 0; i < _maxConnections; ++i)
	{
		if (!_remotePeerSlots[i])
		{
			return i;
		}
	}

	return -1;
}

RemotePeer* Peer::GetRemotePeerFromAddress(const Address& address)
{
	RemotePeer* result = nullptr;
	for (unsigned int i = 0; i < _maxConnections; ++i)
	{
		if (!_remotePeerSlots[i])
		{
			continue;
		}

		if (_remotePeers[i].GetAddress() == address)
		{
			result = &_remotePeers[i];
			break;
		}
	}

	return result;
}

bool Peer::IsRemotePeerAlreadyConnected(const Address& address) const
{
	bool found = false;
	for (unsigned int i = 0; i < _maxConnections; ++i)
	{
		if (!_remotePeerSlots[i])
		{
			continue;
		}

		if (_remotePeers[i].GetAddress() == address)
		{
			found = true;
			break;
		}
	}

	return found;
}

PendingConnection* Peer::GetPendingConnectionFromAddress(const Address& address)
{
	PendingConnection* result = nullptr;
	int index = GetPendingConnectionIndexFromAddress(address);
	if (index != -1)
	{
		result = &_pendingConnections[index];
	}

	return result;
}

void Peer::RemovePendingConnection(const Address& address)
{
	int index = GetPendingConnectionIndexFromAddress(address);
	if (index != -1)
	{
		_pendingConnections.erase(_pendingConnections.begin() + index);
	}
}

bool Peer::IsPendingConnectionAlreadyAdded(const Address& address) const
{
	return GetPendingConnectionIndexFromAddress(address) != -1;
}

bool Peer::BindSocket(const Address& address) const
{
	SocketResult result = _socket.Bind(address);
	if (result != SocketResult::SUCCESS)
	{
		return false;
	}

	return true;
}

bool Peer::InitializeSocketsLibrary()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);//Init WS. You need to pass it the version (1.0, 1.1, 2.2...) and a pointer to WSADATA which contains info about the WS impl.
	if (iResult != 0)
	{
		LOG_ERROR("WSAStartup failed: " + iResult);
		return false;
	}

	return true;
}

void Peer::ProcessReceivedData()
{
	Address remoteAddress = Address::GetInvalid();
	unsigned int numberOfBytesRead = 0;
	bool arePendingDatagramsToRead = true;

	do
	{
		SocketResult result = _socket.ReceiveFrom(_receiveBuffer, _receiveBufferSize, &remoteAddress, numberOfBytesRead);

		if (result == SocketResult::SUCCESS)
		{
			//Data read succesfully. Keep going!
			Buffer buffer = Buffer(_receiveBuffer, numberOfBytesRead);
			ProcessDatagram(buffer, remoteAddress);
		}
		else if (result == SocketResult::ERR || result == SocketResult::WOULDBLOCK)
		{
			//An unexpected error occurred or there is no more data to read atm
			arePendingDatagramsToRead = false;
		}
		else if (result == SocketResult::CONNRESET)
		{
			//The remote socket got closed unexpectedly
			int remotePeerIndex = GetRemotePeerIndexFromAddress(remoteAddress);
			if (remotePeerIndex != -1)
			{
				StartDisconnectingRemotePeer(remotePeerIndex);
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

	RemotePeer* remotePeer = GetRemotePeerFromAddress(address);
	bool isPacketFromRemotePeer = (remotePeer != nullptr);

	//Process packet ACKs
	if(isPacketFromRemotePeer)
	{
		uint32_t acks = packet.GetHeader().ackBits;
		uint16_t lastAckedMessageSequenceNumber = packet.GetHeader().lastAckedSequenceNumber;
		TransmissionChannelType channelType = static_cast<TransmissionChannelType>(packet.GetHeader().channelType);
		remotePeer->ProcessACKs(acks, lastAckedMessageSequenceNumber, channelType);
	}

	//Process packet messages one by one
	std::vector<Message*>::iterator iterator = packet.GetMessages();
	unsigned int numberOfMessagesInPacket = packet.GetNumberOfMessages();
	MessageFactory* messageFactory = MessageFactory::GetInstance();

	for (unsigned int i = 0; i < numberOfMessagesInPacket; ++i)
	{
		Message* message = *(iterator + i);

		if (isPacketFromRemotePeer)
		{
			remotePeer->AddReceivedMessage(message);
		}
		else
		{
			ProcessMessage(*message, address);
			messageFactory->ReleaseMessage(message);
		}
	}
}

void Peer::ProcessNewRemotePeerMessages()
{
	for (unsigned int i = 0; i < _remotePeerSlots.size(); ++i)
	{
		if(!_remotePeerSlots[i])
		{
			continue;
		}

		//Process ready to process messages from remote peer
		RemotePeer& remotePeer = _remotePeers[i];

		while (remotePeer.ArePendingReadyToProcessMessages())
		{
			const Message* message = remotePeer.GetPendingReadyToProcessMessage();
			ProcessMessage(*message, remotePeer.GetAddress());
		}

		remotePeer.FreeProcessedMessages();
	}
}

void Peer::TickRemotePeers(float elapsedTime)
{
	for (unsigned int i = 0; i < _maxConnections; ++i)
	{
		if (_remotePeerSlots[i])
		{
			_remotePeers[i].Tick(elapsedTime);
		}
	}
}

void Peer::HandlerRemotePeersInactivity()
{
	for (unsigned int i = 0; i < _maxConnections; ++i)
	{
		if (_remotePeerSlots[i])
		{
			if (_remotePeers[i].IsInactive())
			{
				StartDisconnectingRemotePeer(i);
			}
		}
	}
}

int Peer::GetPendingConnectionIndexFromAddress(const Address& address) const
{
	int index = -1;
	for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
	{
		if (_pendingConnections[i].GetAddress() == address)
		{
			index = i;
			break;
		}
	}

	return index;
}

int Peer::GetRemotePeerIndexFromAddress(const Address& address) const
{
	int index = -1;
	for (unsigned int i = 0; i < _maxConnections; ++i)
	{
		if (!_remotePeerSlots[i])
		{
			continue;
		}

		if (_remotePeers[i].GetAddress() == address)
		{
			index = i;
			break;
		}
	}

	return index;
}

void Peer::SendData()
{
	for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
	{
		if (!_pendingConnections[i].ArePendingMessages())
		{
			continue;
		}

		NetworkPacket packet = NetworkPacket();
		Message* message;
		do
		{
			message = _pendingConnections[i].GetAMessage();
			packet.AddMessage(message);
		} while (_pendingConnections[i].ArePendingMessages());

		SendPacketToAddress(packet, _pendingConnections[i].GetAddress());

		_pendingConnections[i].FreeSentMessages();
	}

	for (unsigned int i = 0; i < _remotePeerSlots.size(); ++i)
	{
		if (!_remotePeerSlots[i])
		{
			continue;
		}

		SendPacketToRemotePeer(_remotePeers[i], TransmissionChannelType::UnreliableUnordered);
		SendPacketToRemotePeer(_remotePeers[i], TransmissionChannelType::ReliableOrdered);
	}
}

void Peer::SendPacketToRemotePeer(RemotePeer& remotePeer, TransmissionChannelType type)
{
	if (!remotePeer.ArePendingMessages(type) && !remotePeer.AreUnsentACKs(type))
	{
		return;
	}

	NetworkPacket packet = NetworkPacket();
	Message* message = nullptr;

	//TODO Check somewhere if there is a message larger than the maximum packet size. Log a warning saying that the message will never get sent and delete it.
	//TODO Include data prefix in packet's header and check if the data prefix is correct when receiving a packet


	//Check if we should include a message to the packet
	bool arePendingMessages = remotePeer.ArePendingMessages(type);
	bool isThereCapacityLeft = packet.CanMessageFit(remotePeer.GetSizeOfNextUnsentMessage(type));

	while (arePendingMessages && isThereCapacityLeft)
	{
		//Configure and add message to packet
		message = remotePeer.GetPendingMessage(type);

		//uint16_t messageSequenceNumber = remotePeer.GetNextMessageSequenceNumber(type);
		//remotePeer.IncreaseMessageSequenceNumber(type);

		if (message->GetHeader().isReliable)
		{
			std::stringstream ss;
			ss << "Reliable message sequence number: " << message->GetHeader().messageSequenceNumber << " Message type: " << (int)message->GetHeader().type;
			LOG_INFO(ss.str());
		}

		//message->SetHeaderPacketSequenceNumber(messageSequenceNumber);

		packet.AddMessage(message);

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
	//TODO Make this to execute only once per tick and not per channel packet transmission
	remotePeer.FreeSentMessages();
}

void Peer::SendDataToAddress(const Buffer& buffer, const Address& address) const
{
	_socket.SendTo(buffer.GetData(), buffer.GetSize(), address);
}

void Peer::StartDisconnectingRemotePeer(unsigned int index)
{
	DisconnectRemotePeerConcrete(_remotePeers[index]);
	_remotePeerSlotIDsToDisconnect.push(index);
}

void Peer::FinishRemotePeersDisconnection()
{
	unsigned int remoteClientSlot;
	while (!_remotePeerSlotIDsToDisconnect.empty())
	{
		remoteClientSlot = _remotePeerSlotIDsToDisconnect.front();
		_remotePeerSlotIDsToDisconnect.pop();

		if (_remotePeerSlots[remoteClientSlot])
		{
			_remotePeerSlots[remoteClientSlot] = false;
			_remotePeers[remoteClientSlot].Disconnect();
		}
	}
}
