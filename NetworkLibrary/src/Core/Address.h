#pragma once
#include "NumericTypes.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

namespace NetLib
{
	constexpr char* IPV4_ANY = "0.0.0.0";
	constexpr char* IPV4_LOOPBACK = "127.0.0.1";

	enum class IPVersion
	{
		IPV4 = 0,
		IPV6 = 1
	};

	class Address
	{
		private:
			void InitSockAddr();
			// This function is only called from socket class
			void SetFromSockAddr( const sockaddr_in& addressInfo );
			// This function is only called from socket class
			const sockaddr_in& GetSockAddr() const { return _addressInfo; }

			std::string _ip;
			IPVersion _ipVersion;
			uint32 _port;

			struct sockaddr_in _addressInfo;

			friend class Socket;

		public:
			static Address GetInvalid() { return Address( "0.0.0.0", 0 ); }

			Address( const std::string& ip, uint32 port );

			Address( const Address& other ) = default;

			uint32 GetPort() const { return _port; }
			std::string GetIP() const { return _ip; }
			void GetFull( std::string& buffer ) const;

			bool operator==( const Address& other ) const;
	};
} // namespace NetLib
