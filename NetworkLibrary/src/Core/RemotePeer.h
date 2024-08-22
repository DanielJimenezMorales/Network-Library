#pragma once
#include <cstdint>
#include <map>
#include <memory>

#include "Address.h"
#include "TransmissionChannel.h"
#include "Logger.h"

namespace NetLib
{
	class Message;
	struct MessageHeader;

	enum RemotePeerState : uint8_t
	{
		Disconnected = 0,
		Connected = 1,
		Connecting = 2
	};

	class RemotePeer
	{
	private:
		Address _address;
		uint16_t _id = 0;
		RemotePeerState _currentState;

		float _maxInactivityTime;
		float _inactivityTimeLeft;
		uint64_t _clientSalt;
		uint64_t _serverSalt;

		uint16_t _nextPacketSequenceNumber;

		std::vector<TransmissionChannel*> _transmissionChannels;

		void InitTransmissionChannels();
		TransmissionChannel* GetTransmissionChannelFromType(TransmissionChannelType channelType);
		const TransmissionChannel* GetTransmissionChannelFromType(TransmissionChannelType channelType) const;
		TransmissionChannelType GetTransmissionChannelTypeFromHeader(const MessageHeader& messageHeader) const;

	public:
		RemotePeer();
		RemotePeer(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t clientSalt, uint64_t serverSalt);
		RemotePeer(const RemotePeer&) = delete;
		RemotePeer(RemotePeer&& other) = default; //This must be here since Peer.h has a std::vector<RemotePeer> and vector<T> requires T to be MoveAssignable

		RemotePeer& operator=(const RemotePeer&) = delete;
		~RemotePeer();

		uint16_t GetLastMessageSequenceNumberAcked(TransmissionChannelType channelType) const;

		/// <summary>
		/// Initializes all the internal systems. You must call this method before performing any other operation. It is also automatically called in
		/// parameterized ctor
		/// </summary>
		/// <param name="addressInfo">Information about the address of this remote client</param>
		/// <param name="id">Client's unique ID</param>
		/// <param name="maxInactivityTime">Maximum number of seconds without receiving information from this client to consider it "Inactive"</param>
		/// <param name="dataPrefix">Client's data encryption prefix generated during the connection pipeline</param>
		void Connect(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t clientSalt, uint64_t serverSalt);

		void SetConnected() { _currentState = RemotePeerState::Connected; }

		void Tick(float elapsedTime);

		const Address& GetAddress() const { return _address; }
		uint16_t GetClientIndex() const { return _id; }
		uint64_t GetDataPrefix() const { return _clientSalt ^ _serverSalt; } //XOR operation to create the data prefix for all packects from now on
		uint64_t GetClientSalt() const { return _clientSalt; }
		uint64_t GetServerSalt() const { return _serverSalt; }
		RemotePeerState GeturrentState() const { return _currentState; }

		void SetServerSalt(uint64_t newValue) { _serverSalt = newValue; }

		bool IsAddressEqual(const Address& other) const { return other == _address; }
		bool IsInactive() const { return _inactivityTimeLeft == 0.f; }
		bool AddMessage(std::unique_ptr<Message> message);
		bool ArePendingMessages(TransmissionChannelType channelType) const;
		std::unique_ptr<Message> GetPendingMessage(TransmissionChannelType channelType);
		unsigned int GetSizeOfNextUnsentMessage(TransmissionChannelType channelType) const;
		void AddSentMessage(std::unique_ptr<Message> message, TransmissionChannelType channelType);
		void FreeSentMessages();
		void FreeProcessedMessages();
		void SeUnsentACKsToFalse(TransmissionChannelType channelType);
		bool AreUnsentACKs(TransmissionChannelType channelType) const;
		uint32_t GenerateACKs(TransmissionChannelType channelType) const;
		void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber, TransmissionChannelType channelType);
		bool AddReceivedMessage(std::unique_ptr<Message> message);

		bool ArePendingReadyToProcessMessages() const;
		const Message* GetPendingReadyToProcessMessage();

		unsigned int GetRTTMilliseconds() const;

		std::vector<TransmissionChannelType> GetAvailableTransmissionChannelTypes() const;
		unsigned int GetNumberOfTransmissionChannels() const;

		/// <summary>
		/// Disconnect and reset the remote client
		/// </summary>
		void Disconnect();
	};
}
