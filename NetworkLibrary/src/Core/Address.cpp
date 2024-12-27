#include "address.h"

#include "logger.h"

namespace NetLib
{
	// Evaluate if supporting address creation from hostname such as hello.com. In order to do that you will need to use
	// getaddrinfo and create a DNS petition which takes wayyyy longer than using just a normal IP. If so, do it in a
	// separate thread.

	Address::Address( const std::string& ip, uint32 port )
	    : _ip( ip )
	    , _ipVersion( IPVersion::IPV4 )
	    , _port( port )
	    , _addressInfo()
	{
		InitSockAddr();
	}

	bool Address::operator==( const Address& other ) const
	{
		return _ip == other._ip && _port == other._port && _ipVersion == other._ipVersion;
	}

	bool Address::operator!=( const Address& other ) const
	{
		return !( *this == other );
	}

	void Address::GetFull( std::string& buffer ) const
	{
		buffer.append( _ip.c_str() );
		buffer.append( ":" );
		buffer.append( std::to_string( _port ) );
	}

	void Address::InitSockAddr()
	{
		_addressInfo.sin_family = ( _ipVersion == IPVersion::IPV4 ) ? AF_INET : AF_INET6;
		_addressInfo.sin_port = htons( _port );
		const int32 iResult = inet_pton( _addressInfo.sin_family, _ip.c_str(), &_addressInfo.sin_addr );
		if ( iResult == -1 )
		{
			LOG_ERROR( "Error at converting IP string into address. Error code: %d", WSAGetLastError() );
		}
		else if ( iResult == 0 )
		{
			LOG_ERROR( "The IP string: %s is not valid", _ip.c_str() );
		}
	}

	void Address::SetFromSockAddr( const sockaddr_in& addressInfo )
	{
		_addressInfo.sin_family = addressInfo.sin_family;
		_addressInfo.sin_port = addressInfo.sin_port;
		_addressInfo.sin_addr = addressInfo.sin_addr;

		// Check if input is an invalid address
		if ( _addressInfo.sin_family == AF_UNSPEC )
		{
			_ip.assign( "0.0.0.0" );
			_port = 0;
		}
		else
		{
			const uint32 IP_MAX_SIZE = 256;
			char clientIp[ IP_MAX_SIZE ];
			ZeroMemory( clientIp, IP_MAX_SIZE );
			inet_ntop( _addressInfo.sin_family, &_addressInfo.sin_addr, clientIp, IP_MAX_SIZE );
			std::string ipString( clientIp );
			_ip.assign( clientIp );

			_ipVersion = ( _addressInfo.sin_family == AF_INET ) ? IPVersion::IPV4 : IPVersion::IPV6;
			_port = ntohs( _addressInfo.sin_port );
		}
	}
} // namespace NetLib
