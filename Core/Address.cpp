#include "Address.h"
#include <sstream>
#include "../Utils/Logger.h"

Address::Address(const std::string& ip, unsigned int port)
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
