#pragma once
#include "NumericTypes.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

namespace NetLib
{
	class Address
	{
		private:
			struct sockaddr_in _addressInfo;

		public:
			static Address GetInvalid() { return Address( "0.0.0.0", 0 ); }

			Address( const std::string& ip, uint16 port );
			Address( const sockaddr_in& addressInfo );

			const sockaddr_in& GetInfo() const { return _addressInfo; }
			uint16 GetPort() const { return ntohs( _addressInfo.sin_port ); }
			std::string GetIP() const;

			bool operator==( const Address& other ) const;
	};
} // namespace NetLib
