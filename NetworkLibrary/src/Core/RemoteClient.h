#pragma once
#include <vector>
#include <queue>
#include "Address.h"

class Message;

class RemoteClient
{
private:
	Address _address;
	uint16_t _id = 0;

	float _maxInactivityTime;
	float _inactivityTimeLeft;
	uint64_t _dataPrefix;

	std::vector<Message*> _pendingMessages;
	std::queue<Message*> _sentMessages;

public:
	RemoteClient();
	RemoteClient(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix);
	~RemoteClient();

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
	bool ArePendingMessages() const { return !_pendingMessages.empty(); }

	/// <summary>
	/// Get a message from the pending messages to send collection. IMPORTANT: DO NOT FREE THIS MEMORY. Instead, call FreeSentMessages() once the packet
	/// has been sent.
	/// </summary>
	/// <returns></returns>
	Message* GetAMessage();

	/// <summary>
	/// Call this after sending a packet to this client. This will release all the memory related to messages sent (Only if they are non reliable)
	/// </summary>
	void FreeSentMessages();

	/// <summary>
	/// Disconnect and reset the remote client
	/// </summary>
	void Disconnect();
};

