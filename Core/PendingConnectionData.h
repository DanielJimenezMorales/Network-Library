#pragma once
#include <cstdint>
#include "Address.h"

struct PendingConnectionData
{
	Address address;
	uint64_t clientSalt;
	uint64_t serverSalt;

	PendingConnectionData(const Address& addr) : address(Address(addr.GetInfo())), clientSalt(0), serverSalt(0) {}

	uint64_t GetPrefix() { return clientSalt ^ serverSalt; }
};