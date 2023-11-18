#include "RemoteClient.h"

RemoteClient::RemoteClient(const sockaddr_in& addressInfo, uint16_t index, float maxInactivityTime, uint64_t dataPrefix) : 
							_index(index), 
							_maxInactivityTime(maxInactivityTime), 
							_inactivityTimeLeft(maxInactivityTime),
							_dataPrefix(dataPrefix)
{
	_address = new Address(addressInfo);
}

RemoteClient::~RemoteClient()
{
	delete _address;
	_address = nullptr;
}

void RemoteClient::Tick(float elapsedTime)
{
	_inactivityTimeLeft -= elapsedTime;

	if (_inactivityTimeLeft < 0.f)
	{
		_inactivityTimeLeft = 0.f;
	}
}
