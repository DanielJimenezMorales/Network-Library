#pragma once
#include <cstdint>

#include "Address.h"
#include "PeerMessagesHandler.h"

class Message;

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
	bool ArePendingMessages() const { return _messagesHandler.ArePendingMessages(); }
	Message* GetPendingMessage();
	bool ArePendingACKReliableMessages() const { return _messagesHandler.ArePendingACKReliableMessages(); };
	Message* GetPendingACKReliableMessage();
	unsigned int GetSizeOfNextPendingMessage() const { return _messagesHandler.GetSizeOfNextPendingMessage(); };
	void FreeSentMessages();
	uint32_t GenerateACKs() const { return _messagesHandler.GenerateACKs(); };

	/// <summary>
	/// Disconnect and reset the remote client
	/// </summary>
	void Disconnect();
};

