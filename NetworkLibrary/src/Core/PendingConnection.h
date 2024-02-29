#pragma once
#include <cstdint>
#include <vector>
#include <queue>
#include "Address.h"
#include "TransmissionChannel.h"

class Message;

class PendingConnection
{
public:
	PendingConnection(const Address& addr);

	bool ArePendingMessages() const;
	bool AddMessage(Message* message);
	Message* GetAMessage();
	void FreeSentMessages();

	uint64_t GetPrefix() const { return _clientSalt ^ _serverSalt; }
	uint64_t GetClientSalt() const { return _clientSalt; }
	uint64_t GetServerSalt() const { return _serverSalt; }
	const Address& GetAddress() const { return _address; }
	bool IsAddressEqual(const Address& address) const { return this->_address == address; }

	void SetClientSalt(uint64_t newValue) { _clientSalt = newValue; }
	void SetServerSalt(uint64_t newValue) { _serverSalt = newValue; }

	~PendingConnection();

private:
	Address _address;
	uint64_t _clientSalt;
	uint64_t _serverSalt;

	TransmissionChannel* _transmissionChannel;
};