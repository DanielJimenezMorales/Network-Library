#pragma once
#include <cstdint>
#include <map>

#include "Address.h"
#include "TransmissionChannel.h"

class Message;
class MessageHeader;

class RemotePeer
{
private:
	Address _address;
	uint16_t _id = 0;

	float _maxInactivityTime;
	float _inactivityTimeLeft;
	uint64_t _dataPrefix;

	uint16_t _nextPacketSequenceNumber;

	std::map<TransmissionChannelType, TransmissionChannel*> _transmissionChannels;

public:
	RemotePeer();
	RemotePeer(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix);
	~RemotePeer();

	uint16_t GetNextMessageSequenceNumber(TransmissionChannelType channelType) const;
	uint16_t GetLastMessageSequenceNumberAcked(TransmissionChannelType channelType) const;
	void IncreaseMessageSequenceNumber(TransmissionChannelType channelType);

	/// <summary>
	/// Initializes all the internal systems. You must call this method before performing any other operation. It is also automatically called in
	/// parameterized ctor
	/// </summary>
	/// <param name="addressInfo">Information about the address of this remote client</param>
	/// <param name="id">Client's unique ID</param>
	/// <param name="maxInactivityTime">Maximum number of seconds without receiving information from this client to consider it "Inactive"</param>
	/// <param name="dataPrefix">Client's data encryption prefix generated during the connection pipeline</param>
	void Connect(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix);

	void Tick(float elapsedTime);

	const Address& GetAddress() const { return _address; }
	uint16_t GetClientIndex() const { return _id; }
	uint64_t GetDataPrefix() const { return _dataPrefix; }
	bool IsAddressEqual(const Address& other) const { return other == _address; }
	bool IsInactive() const { return _inactivityTimeLeft == 0.f; }
	bool AddMessage(Message* message);
	TransmissionChannelType GetTransmissionChannelTypeFromHeader(const MessageHeader& messageHeader) const;
	bool ArePendingMessages(TransmissionChannelType channelType) const;
	Message* GetPendingMessage(TransmissionChannelType channelType);
	unsigned int GetSizeOfNextUnsentMessage(TransmissionChannelType channelType) const;
	void FreeSentMessages();
	void FreeProcessedMessages();
	void SeUnsentACKsToFalse(TransmissionChannelType channelType);
	bool AreUnsentACKs(TransmissionChannelType channelType) const;
	uint32_t GenerateACKs(TransmissionChannelType channelType) const;
	void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber, TransmissionChannelType channelType);
	bool AddReceivedMessage(Message* message);

	bool ArePendingReadyToProcessMessages() const;
	const Message* GetPendingReadyToProcessMessage();

	bool GetNumberOfTransmissionChannels() const;

	/// <summary>
	/// Disconnect and reset the remote client
	/// </summary>
	void Disconnect();
};

