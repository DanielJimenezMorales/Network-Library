#pragma once
#include <queue>
#include "Message.h"

class MessageFactory
{
public:
	static MessageFactory* GetInstance(unsigned int size = 2);

	Message* LendMessage(MessageType messageType);
	void ReleaseMessage(Message* message);

	~MessageFactory();

private:
	MessageFactory(unsigned int size);
	MessageFactory(const MessageFactory&) = delete;

	MessageFactory& operator=(const MessageFactory&) = delete;

	void InitializePools();
	void InitializePool(std::queue<Message*>& pool, MessageType messageType);
	std::queue<Message*>* GetPoolFromType(MessageType messageType);
	Message* CreateMessage(MessageType messageType);
	void ReleasePool(std::queue<Message*>& pool);

	static MessageFactory* _instance;

	bool _isInitialized;
	unsigned int _initialSize;

	std::queue<Message*> _connectionRequestMessagePool;
	std::queue<Message*> _connectionChallengeMessagePool;
	std::queue<Message*> _connectionChallengeResponseMessagePool;
	std::queue<Message*> _connectionAcceptedMessagePool;
	std::queue<Message*> _connectionDeniedMessagePool;
	std::queue<Message*> _disconnectionMessagePool;
};

