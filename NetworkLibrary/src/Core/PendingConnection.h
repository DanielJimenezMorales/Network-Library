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
		PendingConnection();
		PendingConnection(const PendingConnection&) = delete;
		PendingConnection(PendingConnection&& other) noexcept;

		PendingConnection& operator=(const PendingConnection&) = delete;
		PendingConnection& operator=(PendingConnection&& other) noexcept;

		void Initialize(const Address& addr, float timeoutSeconds);

		void Tick(float deltaTime);

		bool IsInactive() const { return _timeoutLeft == 0.f; }

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

		void Reset();

		~PendingConnection();

	private:
		Address _address;
		uint64_t _clientSalt;
		uint64_t _serverSalt;
		float _maxTimeout;
		float _timeoutLeft;

		TransmissionChannel* _transmissionChannel;
	};
}
