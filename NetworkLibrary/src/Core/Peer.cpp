#include <sstream>

#include "Peer.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "Logger.h"
#include "Buffer.h"

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
	if (IsThereNewDataToProcess())
	{
		ProcessReceivedData();
	}

	TickConcrete(elapsedTime);
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
}

void Peer::SendPacketToAddress(const NetworkPacket& packet, const Address& address) const
{
	Buffer* buffer = new Buffer(packet.Size());
	packet.Write(*buffer);

	SendDataToAddress(*buffer, address);

	delete buffer;
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

bool Peer::IsThereNewDataToProcess() const
{
	return _socket.ArePendingDataToReceive();
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

void Peer::SendDataToAddress(const Buffer& buffer, const Address& address) const
{
	_socket.SendTo(buffer.GetData(), buffer.GetSize(), address);
}
