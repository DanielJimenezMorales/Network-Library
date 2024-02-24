#pragma once
#include <cstdint>
#include <map>

#include "Address.h"
#include "PeerMessagesHandler.h"
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

	PeerMessagesHandler _messagesHandler;
	std::map<TransmissionChannelType, TransmissionChannel*> _transmissionChannels;

public:
	RemotePeer();
	RemotePeer(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix);
	~RemotePeer();

	uint16_t GetNextMessageSequenceNumber() const { return _nextPacketSequenceNumber; };
	uint16_t GetLastMessageSequenceNumberAcked() const { return _messagesHandler.GetLastMessageSequenceNumberAcked(); };
	void IncreaseMessageSequenceNumber() { ++_nextPacketSequenceNumber; }

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
	bool ArePendingMessages() const { return _messagesHandler.ArePendingMessages(); }//This will get unuseful
	bool ArePendingMessages(TransmissionChannelType channelType) const;
	Message* GetPendingMessage();//This will get unuseful
	Message* GetPendingMessage(TransmissionChannelType channelType);//This will get unuseful
	bool ArePendingACKReliableMessages() const { return _messagesHandler.AreUnackedReliableMessages(); };
	Message* GetPendingACKReliableMessage();
	unsigned int GetSizeOfNextPendingMessage() const { return _messagesHandler.GetSizeOfNextPendingMessage(); };
	void FreeSentMessages();
	void FreeProcessedMessages();
	uint32_t GenerateACKs() const { return _messagesHandler.GenerateACKs(); };//This will get unuseful
	uint32_t GenerateACKs(TransmissionChannelType channelType) const;
	void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber) { _messagesHandler.ProcessACKs(acks, lastAckedMessageSequenceNumber); };//This will get unuseful
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

