#pragma once
#include <vector>
#include <queue>

class Message;

class PeerMessagesHandler
{
public:
	PeerMessagesHandler();

	/// <summary>
	/// Add a message to the pending messages to send collection.
	/// </summary>
	/// <param name="message">The message to send</param>
	/// <returns>True if the operation succeed</returns>
	bool AddMessage(Message* message);
	bool ArePendingMessages() const { return !_pendingMessages.empty(); }

	/// <summary>
	/// Get a message from the pending messages to send collection. IMPORTANT: DO NOT FREE THIS MEMORY. Instead, call FreeSentMessages() once the packet
	/// has been sent.
	/// </summary>
	/// <returns>A pointer to a pending message to send</returns>
	Message* GetAMessage();

	/// <summary>
	/// Call this after sending a packet to this client. This will release all the memory related to messages sent (Only if they are non reliable)
	/// </summary>
	void FreeSentMessages();
	void ClearMessages();

	~PeerMessagesHandler();

private:
	std::vector<Message*> _pendingMessages;
	std::queue<Message*> _sentMessages;
};

