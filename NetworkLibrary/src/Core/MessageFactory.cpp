#include <cassert>
#include <sstream>
#include "MessageFactory.h"
#include "Logger.h"

MessageFactory* MessageFactory::_instance = nullptr;

MessageFactory* MessageFactory::GetInstance(unsigned int size)
{
    if (_instance == nullptr)
    {
        _instance = new MessageFactory(size);
    }

    return _instance;
}

MessageFactory::MessageFactory(unsigned int size)
{
    _initialSize = size;
    InitializePools();
    _isInitialized = true;
}

Message* MessageFactory::GetMessage(MessageType messageType)
{
    assert(_isInitialized == true);

    Message* message = nullptr;

    std::queue<Message*>* pool = GetPoolFromType(messageType);
    if (pool == nullptr)
    {
        return nullptr;
    }

    if (!pool->empty())
    {
        message = pool->front();
        pool->pop();
    }
    else
    {
        std::stringstream ss;
        ss << "The message pool of type " << messageType << " is empty. Creating a new message... Consider increasing pool size. Current init size: " << _initialSize;
        LOG_WARNING(ss.str());

        message = CreateMessage(messageType);
    }

    assert(message != nullptr);
    assert(message->header.type == messageType);

    return message;
}

void MessageFactory::ReleaseMessage(Message* message)
{
    assert(_isInitialized == true);

    if (message == nullptr)
    {
        return;
    }

    MessageType messageType = message->header.type;
    std::queue<Message*>* pool = GetPoolFromType(messageType);
    if (pool != nullptr)
    {
        pool->push(message);
    }
    else
    {
        delete message;
    }
}

MessageFactory::~MessageFactory()
{
    ReleasePool(_connectionRequestMessagePool);
    ReleasePool(_connectionChallengeMessagePool);
    ReleasePool(_connectionChallengeResponseMessagePool);
    ReleasePool(_connectionAcceptedMessagePool);
    ReleasePool(_connectionDeniedMessagePool);
    ReleasePool(_disconnectionMessagePool);
}

void MessageFactory::InitializePools()
{
    InitializePool(_connectionRequestMessagePool, MessageType::ConnectionRequest);
    InitializePool(_connectionChallengeMessagePool, MessageType::ConnectionChallenge);
    InitializePool(_connectionChallengeResponseMessagePool, MessageType::ConnectionChallengeResponse);
    InitializePool(_connectionAcceptedMessagePool, MessageType::ConnectionAccepted);
    InitializePool(_connectionDeniedMessagePool, MessageType::ConnectionDenied);
    InitializePool(_disconnectionMessagePool, MessageType::Disconnection);
}

void MessageFactory::InitializePool(std::queue<Message*>& pool, MessageType messageType)
{
    for (unsigned int i = 0; i < _initialSize; ++i)
    {
        Message* message = nullptr;
        message = CreateMessage(messageType);
        assert(message != nullptr);
        assert(message->header.type == messageType);

        pool.push(message);
    }
}

std::queue<Message*>* MessageFactory::GetPoolFromType(MessageType messageType)
{
    std::queue<Message*>* resultPool = nullptr;
    switch (messageType)
    {
    case MessageType::ConnectionRequest:
        resultPool = &_connectionRequestMessagePool;
        break;
    case MessageType::ConnectionChallenge:
        resultPool = &_connectionChallengeMessagePool;
        break;
    case MessageType::ConnectionChallengeResponse:
        resultPool = &_connectionChallengeResponseMessagePool;
        break;
    case MessageType::ConnectionAccepted:
        resultPool = &_connectionAcceptedMessagePool;
        break;
    case MessageType::ConnectionDenied:
        resultPool = &_connectionDeniedMessagePool;
        break;
    case MessageType::Disconnection:
        resultPool = &_disconnectionMessagePool;
        break;
    default:
        LOG_ERROR("Can't get desired pool. Invalid message type");
        break;
    }

    return resultPool;
}

Message* MessageFactory::CreateMessage(MessageType messageType)
{
    Message* resultMessage = nullptr;

    switch (messageType)
    {
    case MessageType::ConnectionRequest:
        resultMessage = new ConnectionRequestMessage();
        break;
    case MessageType::ConnectionChallenge:
        resultMessage = new ConnectionChallengeMessage();
        break;
    case MessageType::ConnectionChallengeResponse:
        resultMessage = new ConnectionChallengeResponseMessage();
        break;
    case MessageType::ConnectionAccepted:
        resultMessage = new ConnectionAcceptedMessage();
        break;
    case MessageType::ConnectionDenied:
        resultMessage = new ConnectionDeniedMessage();
        break;
    case MessageType::Disconnection:
        resultMessage = new DisconnectionMessage();
        break;
    default:
        LOG_ERROR("Can't create a new message. Invalid message type");
        break;
    }

    return resultMessage;
}

void MessageFactory::ReleasePool(std::queue<Message*>& pool)
{
    Message* messageToDelete = nullptr;
    while (!pool.empty())
    {
        messageToDelete = pool.front();
        pool.pop();
        delete messageToDelete;
        messageToDelete = nullptr;
    }
}
