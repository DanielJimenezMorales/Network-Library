#include "Address.h"
#include "Logger.h"

namespace NetLib
{
	//Evaluate if supporting address creation from hostname such as hello.com. In order to do that you will need to use getaddrinfo
	//and create a DNS petition which takes wayyyy longer than using just a normal IP. If so, do it in a separate thread.

	Address::Address(const std::string& ip, uint16_t port)
	{
		_addressInfo.sin_family = AF_INET;
		_addressInfo.sin_port = htons(port);
		int iResult = inet_pton(AF_INET, ip.c_str(), &_addressInfo.sin_addr);
		if (iResult == -1)
		{
			LOG_ERROR("Error at converting IP string into address. Error code: %d", WSAGetLastError());
		}
		else if (iResult == 0)
		{
			LOG_ERROR("The IP string: %s is not valid", ip.c_str());
		}
	}

	Address::Address(const sockaddr_in& addressInfo)
	{
		_addressInfo.sin_family = addressInfo.sin_family;
		_addressInfo.sin_port = addressInfo.sin_port;
		_addressInfo.sin_addr = addressInfo.sin_addr;
	}

	std::string Address::GetIP() const
	{
		char clientIp[256];
		ZeroMemory(clientIp, 256);
		inet_ntop(_addressInfo.sin_family, &_addressInfo.sin_addr, clientIp, 256);
		std::string ipString(clientIp);
		return ipString;
	}

	bool Address::operator==(const Address& other) const
	{
		if (_addressInfo.sin_family == other._addressInfo.sin_family &&
			_addressInfo.sin_port == other._addressInfo.sin_port &&
			_addressInfo.sin_addr.S_un.S_addr == other._addressInfo.sin_addr.S_un.S_addr)
		{
			return true;
		}

		return false;
	}
}
