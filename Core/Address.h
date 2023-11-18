#pragma once
#include <cstdint>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include "../Utils/Logger.h"

class Address
{
private:
	struct sockaddr_in _addressInfo;

public:
	Address(const std::string& ip, unsigned int port)
	{
		_addressInfo.sin_family = AF_INET;
		_addressInfo.sin_port = htons(port);
		int iResult = inet_pton(AF_INET, ip.c_str(), &_addressInfo.sin_addr);
		if (iResult == -1)
		{
			LOG_ERROR("Error at converting IP string into address. Error code: " + WSAGetLastError());
		}
		else if (iResult == 0)
		{
			LOG_ERROR("The IP string: " + ip + " is not valid");
		}
	}

	Address(const sockaddr_in& addressInfo)
	{
		_addressInfo.sin_family = addressInfo.sin_family;
		_addressInfo.sin_port = addressInfo.sin_port;
		_addressInfo.sin_addr = addressInfo.sin_addr;
	}

	const sockaddr_in& GetInfo() const { return _addressInfo; }
	unsigned short GetPort() const { return ntohs(_addressInfo.sin_port); }
	std::string GetIP() const
	{
		char clientIp[256];
		ZeroMemory(clientIp, 256);
		inet_ntop(_addressInfo.sin_family, &_addressInfo.sin_addr, clientIp, 256);
		std::string ipString(clientIp);
		return ipString;
	}

	bool operator==(const Address& other) const;
};
