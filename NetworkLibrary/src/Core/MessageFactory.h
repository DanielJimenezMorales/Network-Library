#pragma once
#include <queue>
#include "Message.h"

class MessageFactory
{
public:
	static void Initialize(unsigned int size);
	static Message* GetMessage(MessageType messageType);
	static void ReleaseMessage(Message* message);

	//~MessageFactory();

private:
	static void InitializePools();
	static void InitializePool(std::queue<Message*>& pool, MessageType messageType);
	static std::queue<Message*>* GetPoolFromType(MessageType messageType);
	static Message* CreateMessage(MessageType messageType);
	static void ReleasePool(std::queue<Message*>& pool);

	static bool _isInitialized;
	static unsigned int _initialSize;

	static std::queue<Message*> _connectionRequestMessagePool;
	static std::queue<Message*> _connectionChallengeMessagePool;
	static std::queue<Message*> _connectionChallengeResponseMessagePool;
	static std::queue<Message*> _connectionAcceptedMessagePool;
	static std::queue<Message*> _connectionDeniedMessagePool;
	static std::queue<Message*> _disconnectionMessagePool;
};

