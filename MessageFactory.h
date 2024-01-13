#pragma once
#include <queue>
#include "Message.h"

class MessageFactory
{
public:
	MessageFactory(unsigned int size);

	Message* GetMessage(MessageType messageType);
	void ReleaseMessage(Message* message);

	~MessageFactory();

private:
	void InitializePools();
	void InitializePool(std::queue<Message*>& pool, MessageType messageType);
	std::queue<Message*>& GetPoolFromType(MessageType messageType);
	Message* CreateMessage(MessageType messageType);
	void ReleasePool(std::queue<Message*>& pool);

	unsigned int _initialSize;

	std::queue<Message*> _connectionRequestMessagePool;
	std::queue<Message*> _connectionChallengeMessagePool;
	std::queue<Message*> _connectionChallengeResponseMessagePool;
	std::queue<Message*> _connectionAcceptedMessagePool;
	std::queue<Message*> _connectionDeniedMessagePool;
	std::queue<Message*> _disconnectionMessagePool;
};

