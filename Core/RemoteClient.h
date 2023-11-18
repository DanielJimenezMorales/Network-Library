#pragma once
#include "Address.h"

class RemoteClient
{
private:
	Address* _address;
	uint16_t _index = 0;

	const float _maxInactivityTime;
	float _inactivityTimeLeft;
	const uint64_t _dataPrefix;

public:
	RemoteClient(const sockaddr_in& addressInfo, uint16_t index, float maxInactivityTime, uint64_t dataPrefix);
	~RemoteClient();

	void Tick(float elapsedTime);

	const Address& GetAddress() const { return *_address; }
	uint16_t GetClientIndex() const { return _index; }
	uint64_t GetDataPrefix() const { return _dataPrefix; }
	bool IsAddressEqual(const Address& other) const { return other == *_address; }
	bool IsInactive() const { return _inactivityTimeLeft == 0.f; }
};

