#include <sstream>

#include "Peer.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "Logger.h"
#include "Buffer.h"
#include "RemotePeer.h"

#define SERVER_PORT 54000

bool Peer::Start()
{
	if (!InitializeSocketsLibrary())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}

	if (!_socket.Start())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}

	if (!BindSocket())
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
	if (_socket.ArePendingDataToReceive()) //TODO delete this and add it inside ProcessReceiveData. Create buffers to reuse
	{
		ProcessReceivedData();
	}

	TickConcrete(elapsedTime);

	SendData();
	FinishRemoteClientsDisconnection(); //Change name to Remote Peer instead of Remote Client
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
	_remoteClientSlots.clear();
	_remoteClients.clear();
}

Peer::Peer(PeerType type, int maxConnections) : _type(type), _socket(), _address(Address::GetInvalid()), _maxConnections(maxConnections)
{
	if (_maxConnections > 0)
	{
		_remoteClientSlots.reserve(_maxConnections);
		_remoteClients.reserve(_maxConnections);
		_pendingConnections.reserve(_maxConnections * 2); //There could be more pending connections than clients

		for (size_t i = 0; i < _maxConnections; ++i)
		{
			_remoteClientSlots.push_back(false);
			_remoteClients.push_back(RemotePeer());
		}
	}
}

void Peer::SendPacketToAddress(const NetworkPacket& packet, const Address& address) const
{
	Buffer* buffer = new Buffer(packet.Size());
	packet.Write(*buffer);

	SendDataToAddress(*buffer, address);

	delete buffer;
}

bool Peer::AddRemoteClient(const Address& addressInfo, uint16_t id, uint64_t dataPrefix)
{
	int slotIndex = FindFreeRemoteClientSlot();
	if(slotIndex == -1)
	{
		return false;
	}
	
	_remoteClientSlots[slotIndex] = true;
	_remoteClients[slotIndex].Connect(addressInfo.GetInfo(), id, REMOTE_CLIENT_INACTIVITY_TIME, dataPrefix);
	return true;
}

int Peer::FindFreeRemoteClientSlot() const
{
	for (int i = 0; i < _maxConnections; ++i)
	{
		if (!_remoteClientSlots[i])
		{
			return i;
		}
	}

	return -1;
}

RemotePeer* Peer::GetRemoteClientFromAddress(const Address& address)
{
	RemotePeer* result = nullptr;
	for (unsigned int i = 0; i < _maxConnections; ++i)
	{
		if (!_remoteClientSlots[i])
		{
			continue;
		}

		if (_remoteClients[i].GetAddress() == address)
		{
			result = &_remoteClients[i];
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
		if (!_remoteClientSlots[i])
		{
			continue;
		}

		if (_remoteClients[i].GetAddress() == address)
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

bool Peer::BindSocket()
{
	//Me he quedado aquí mirando para que es el bind y por qué solo se hace en el servidor. Mirar quicknet.
	//Me interesa meter la lista de remote clients directamente en peer en vez de tenerlo solo en server. El cliente también tendrá
	//dicha lista pero será igual a 1 (el servidor). Así puedo soportar también P2P

	if (_type == PeerType::ServerMode)
	{
		sockaddr_in serverHint;
		serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
		serverHint.sin_family = AF_INET;
		serverHint.sin_port = htons(SERVER_PORT); // Convert from little to big endian
		Address address = Address(serverHint);
		_socket.Bind(address);
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
	Buffer* buffer = nullptr;
	Address address = Address::GetInvalid();
	GetDatagramFromAddress(&buffer, &address);
	ProcessDatagram(*buffer, address);

	delete buffer;
	buffer = nullptr;
}

bool Peer::GetDatagramFromAddress(Buffer** buffer, Address* address)
{
	int size = 1024;
	uint8_t* data = new uint8_t[size];
	unsigned int numberOfBytesRead = 0;

	bool result = _socket.ReceiveFrom(data, size, address, numberOfBytesRead);
	if (!result)
	{
		return false;
	}

	*buffer = new Buffer(data, numberOfBytesRead);
	return true;
}

void Peer::ProcessDatagram(Buffer& buffer, const Address& address)
{
	//Read incoming packet
	NetworkPacket packet = NetworkPacket();
	packet.Read(buffer);

	if (_type == PeerType::ServerMode)
	{
		std::stringstream ss;
		ss << "Packet received. Sequence number equal to " << packet.GetHeader().sequenceNumber;
		LOG_INFO(ss.str());
	}

	//Process packet messages one by one
	std::vector<Message*>::const_iterator constIterator = packet.GetMessages();
	unsigned int numberOfMessagesInPacket = packet.GetNumberOfMessages();
	const Message* message = nullptr;
	for (unsigned int i = 0; i < numberOfMessagesInPacket; ++i)
	{
		message = *(constIterator + i);
		ProcessMessage(*message, address);
	}

	//Free memory for those messages in the packet.Read() operation
	packet.ReleaseMessages();
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

void Peer::SendData()
{
	for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
	{
		if (!_pendingConnections[i].ArePendingMessages())
		{
			continue;
		}

		NetworkPacket packet = NetworkPacket(0);
		Message* message;
		do
		{
			message = _pendingConnections[i].GetAMessage();
			packet.AddMessage(message);
		} while (_pendingConnections[i].ArePendingMessages());

		SendPacketToAddress(packet, _pendingConnections[i].GetAddress());

		_pendingConnections[i].FreeSentMessages();
	}

	for (unsigned int i = 0; i < _remoteClientSlots.size(); ++i)
	{
		if (!_remoteClientSlots[i])
		{
			continue;
		}

		if (!_remoteClients[i].ArePendingMessages())
		{
			continue;
		}

		NetworkPacket packet = NetworkPacket(0);
		Message* message;
		do
		{
			message = _remoteClients[i].GetAMessage();
			packet.AddMessage(message);
		} while (_remoteClients[i].ArePendingMessages());

		Buffer* buffer = new Buffer(packet.Size());
		packet.Write(*buffer);
		SendPacketToRemoteClient(_remoteClients[i], packet);
		delete buffer;

		_remoteClients[i].FreeSentMessages();
	}
}

void Peer::SendPacketToRemoteClient(const RemotePeer& remoteClient, const NetworkPacket& packet) const
{
	SendPacketToAddress(packet, remoteClient.GetAddress());
}

void Peer::SendDataToAddress(const Buffer& buffer, const Address& address) const
{
	_socket.SendTo(buffer.GetData(), buffer.GetSize(), address);
}

void Peer::FinishRemoteClientsDisconnection()
{
	unsigned int remoteClientSlot;
	while (!_remoteClientSlotIDsToDisconnect.empty())
	{
		remoteClientSlot = _remoteClientSlotIDsToDisconnect.front();
		_remoteClientSlotIDsToDisconnect.pop();

		if (_remoteClientSlots[remoteClientSlot])
		{
			_remoteClientSlots[remoteClientSlot] = false;
			_remoteClients[remoteClientSlot].Disconnect();
		}
	}
}
