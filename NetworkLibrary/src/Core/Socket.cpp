#include "socket.h"

#include "core/address.h"

#include "logger.h"

namespace NetLib
{
	Socket::Socket()
	    : _listenSocket( INVALID_SOCKET )
	{
	}

	SocketResult Socket::InitializeSocketsLibrary()
	{
		WSADATA wsaData;
		int32 iResult =
		    WSAStartup( MAKEWORD( 2, 2 ), &wsaData ); // Init WS. You need to pass it the version (1.0, 1.1, 2.2...) and
		                                              // a pointer to WSADATA which contains info about the WS impl.
		if ( iResult != 0 )
		{
			LOG_ERROR( "WSAStartup failed: %d", iResult );
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	int32 Socket::GetLastError() const
	{
		return WSAGetLastError();
	}

	bool Socket::IsValid() const
	{
		return !( _listenSocket == INVALID_SOCKET );
	}

	SocketResult Socket::SetBlockingMode( bool status )
	{
		if ( !IsValid() )
		{
			return SocketResult::SOKT_ERR;
		}

		unsigned long listenSocketBlockingMode = status ? 0 : 1;
		const int32 iResult = ioctlsocket( _listenSocket, FIONBIO, &listenSocketBlockingMode );
		if ( iResult == SOCKET_ERROR )
		{
			LOG_ERROR( "Socket error. Error while setting blocking mode, to %lu. Error code %d",
			           listenSocketBlockingMode, GetLastError() );
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::Create()
	{
		_listenSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if ( !IsValid() )
		{
			LOG_ERROR( "Socket error. Error while creating the socket. Error code %d", GetLastError() );
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::Bind( const Address& address ) const
	{
		if ( !IsValid() )
		{
			return SocketResult::SOKT_ERR;
		}

		// If address port is 0 this function will pick up a random port number
		const int32 iResult =
		    bind( _listenSocket, ( sockaddr* ) &address.GetSockAddr(), sizeof( address.GetSockAddr() ) );
		if ( iResult == SOCKET_ERROR )
		{
			LOG_ERROR( "Socket error. Error while binding the listen socket. Error code %d", GetLastError() );
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::Close()
	{
		if ( !IsValid() )
		{
			return SocketResult::SOKT_ERR;
		}

		int32 iResult = closesocket( _listenSocket );
		if ( iResult == SOCKET_ERROR )
		{
			LOG_ERROR( "Socket error. Error while closing the socket. Error code %d", GetLastError() );
			return SocketResult::SOKT_ERR;
		}

		iResult = WSACleanup();
		if ( iResult == SOCKET_ERROR )
		{
			LOG_ERROR( "Socket error. Error while closing the sockets library. Error code %d", GetLastError() );
			return SocketResult::SOKT_ERR;
		}

		_listenSocket = INVALID_SOCKET;

		LOG_INFO( "Socket succesfully closed" );
		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::Start()
	{
		SocketResult result = SocketResult::SOKT_SUCCESS;

		result = InitializeSocketsLibrary();
		if ( result != SocketResult::SOKT_SUCCESS )
		{
			LOG_ERROR( "Error while starting the sockets library, aborting operation..." );
			return result;
		}

		result = Create();
		if ( result != SocketResult::SOKT_SUCCESS )
		{
			return result;
		}

		result = SetBlockingMode( false );
		if ( result != SocketResult::SOKT_SUCCESS )
		{
			return result;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::ReceiveFrom( uint8* incomingDataBuffer, uint32 incomingDataBufferSize, Address& remoteAddress,
	                                  uint32& numberOfBytesRead ) const
	{
		if ( incomingDataBuffer == nullptr || !IsValid() )
		{
			return SocketResult::SOKT_ERR;
		}

		sockaddr_in incomingAddress;
		int32 incomingAddressSize = sizeof( incomingAddress );
		ZeroMemory( &incomingAddress, incomingAddressSize );

		const int32 bytesIn = recvfrom( _listenSocket, ( char* ) incomingDataBuffer, incomingDataBufferSize, 0,
		                                ( sockaddr* ) &incomingAddress, &incomingAddressSize );

		remoteAddress.SetFromSockAddr( incomingAddress );

		if ( bytesIn == SOCKET_ERROR )
		{
			const int32 error = GetLastError();

			if ( error == WSAEMSGSIZE )
			{
				LOG_ERROR( "Socket error. The message received does not fit inside the buffer." );
				return SocketResult::SOKT_ERR;
			}
			else if ( error == WSAEWOULDBLOCK )
			{
				return SocketResult::SOKT_WOULDBLOCK;
			}
			else if ( error == WSAECONNRESET )
			{
				LOG_WARNING( "Socket warning. The remote socket has been closed unexpectly." );
				return SocketResult::SOKT_CONNRESET;
			}
			else
			{
				LOG_ERROR( "Socket error. Error while receiving a message. Error code: %d", error );
				return SocketResult::SOKT_ERR;
			}
		}

		numberOfBytesRead = bytesIn;

		std::string ip_and_port;
		remoteAddress.GetFull( ip_and_port );
		LOG_INFO( "Socket info. Data received from %s", ip_and_port.c_str() );

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::SendTo( const uint8* dataBuffer, uint32 dataBufferSize, const Address& remoteAddress ) const
	{
		if ( dataBuffer == nullptr || !IsValid() )
		{
			return SocketResult::SOKT_ERR;
		}

		if ( dataBufferSize > MTU_SIZE_BYTES )
		{
			LOG_WARNING( "Socket warning. Trying to send a packet bigger than the MTU size theshold. This could result "
			             "in Packet Fragmentation and as a consequence worse network conditions. Packet size: %u, MTU "
			             "size threshold: %u",
			             dataBufferSize, MTU_SIZE_BYTES );
		}

		const int32 bytesSent =
		    sendto( _listenSocket, ( char* ) dataBuffer, dataBufferSize, 0, ( sockaddr* ) &remoteAddress.GetSockAddr(),
		            sizeof( remoteAddress.GetSockAddr() ) );
		if ( bytesSent == SOCKET_ERROR )
		{
			LOG_ERROR( "Socket error. Error while sending data. Error code %d", GetLastError() );
			return SocketResult::SOKT_ERR;
		}

		std::string ip_and_port;
		remoteAddress.GetFull( ip_and_port );
		LOG_INFO( "Socket info. Data sent to %s", ip_and_port.c_str() );

		return SocketResult::SOKT_SUCCESS;
	}

	Socket::~Socket()
	{
		Close();
	}
} // namespace NetLib
