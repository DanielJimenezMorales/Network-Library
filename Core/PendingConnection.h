#pragma once
#include <cstdint>
#include <vector>
#include "Address.h"

class Message;

class PendingConnection
{
public:
	PendingConnection(const Address& addr);

	bool ArePendingMessages() const { return !_pendingMessages.empty(); }
	bool AddMessage(Message* message);
	Message* GetAMessage();

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

	std::vector<Message*> _pendingMessages;
};