#pragma once
#include <cstdint>
#include <vector>
#include <queue>
#include <memory>

#include "Address.h"
#include "TransmissionChannel.h"

namespace NetLib
{
	class Message;

	class PendingConnection
	{
	public:
		PendingConnection(const Address& addr);
		PendingConnection(const PendingConnection&) = delete;
		PendingConnection(PendingConnection&& other) noexcept;

		PendingConnection& operator=(const PendingConnection&) = delete;
		PendingConnection& operator=(PendingConnection&& other) noexcept;

		bool ArePendingMessages() const;
		bool AddMessage(std::unique_ptr<Message> message);
		std::unique_ptr<Message> GetAMessage();
		void AddSentMessage(std::unique_ptr<Message> message);
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
}
