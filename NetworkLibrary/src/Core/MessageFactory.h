#pragma once
#include <queue>
#include <unordered_map>

#include "Message.h"

class MessageFactory
{
public:
	static void CreateInstance(unsigned int size);

	/// <summary>
	/// Get unique instance. Before calling to this method, be sure to call CreateInstance(unsigned int size) or you will get an error.
	/// </summary>
	/// <returns></returns>
	static MessageFactory& GetInstance();

	Message* LendMessage(MessageType messageType);
	void ReleaseMessage(Message* message);

	static void DeleteInstance();

private:
	MessageFactory(unsigned int size);
	MessageFactory(const MessageFactory&) = delete;

	MessageFactory& operator=(const MessageFactory&) = delete;

	void InitializePools();
	void InitializePool(std::queue<Message*>& pool, MessageType messageType);
	std::queue<Message*>* GetPoolFromType(MessageType messageType);
	Message* CreateMessage(MessageType messageType);
	void ReleasePool(std::queue<Message*>& pool);

	~MessageFactory();

	static MessageFactory* _instance;

	bool _isInitialized;
	unsigned int _initialSize;

	std::unordered_map<MessageType, std::queue<Message*>> _messagePools;
};

