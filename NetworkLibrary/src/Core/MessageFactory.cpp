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

Message* MessageFactory::LendMessage(MessageType messageType)
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
        ss << "The message pool of type " << (unsigned int)messageType << " is empty. Creating a new message... Consider increasing pool size. Current init size: " << _initialSize;
        LOG_WARNING(ss.str());

        message = CreateMessage(messageType);
    }

    assert(message != nullptr);
    assert(message->GetHeader().type == messageType);

    return message;
}

void MessageFactory::ReleaseMessage(Message* message)
{
    assert(_isInitialized == true);
    assert(message != nullptr);

    MessageType messageType = message->GetHeader().type;
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
    for (std::unordered_map<MessageType, std::queue<Message*>>::iterator it = _messagePools.begin(); it != _messagePools.end(); ++it)
    {
        ReleasePool((*it).second);
    }

    _messagePools.clear();
}

void MessageFactory::InitializePools()
{
    _messagePools[MessageType::ConnectionRequest] = std::queue<Message*>();
    InitializePool(_messagePools[MessageType::ConnectionRequest], MessageType::ConnectionRequest);

    _messagePools[MessageType::ConnectionChallenge] = std::queue<Message*>();
    InitializePool(_messagePools[MessageType::ConnectionChallenge], MessageType::ConnectionChallenge);

    _messagePools[MessageType::ConnectionChallengeResponse] = std::queue<Message*>();
    InitializePool(_messagePools[MessageType::ConnectionChallengeResponse], MessageType::ConnectionChallengeResponse);

    _messagePools[MessageType::ConnectionAccepted] = std::queue<Message*>();
    InitializePool(_messagePools[MessageType::ConnectionAccepted], MessageType::ConnectionAccepted);
    
    _messagePools[MessageType::ConnectionDenied] = std::queue<Message*>();
    InitializePool(_messagePools[MessageType::ConnectionDenied], MessageType::ConnectionDenied);

    _messagePools[MessageType::Disconnection] = std::queue<Message*>();
    InitializePool(_messagePools[MessageType::Disconnection], MessageType::Disconnection);

    _messagePools[MessageType::InGame] = std::queue<Message*>();
    InitializePool(_messagePools[MessageType::InGame], MessageType::InGame);

    _messagePools[MessageType::InGameResponse] = std::queue<Message*>();
    InitializePool(_messagePools[MessageType::InGameResponse], MessageType::InGameResponse);
}

void MessageFactory::InitializePool(std::queue<Message*>& pool, MessageType messageType)
{
    for (unsigned int i = 0; i < _initialSize; ++i)
    {
        Message* message = nullptr;
        message = CreateMessage(messageType);
        assert(message != nullptr);
        assert(message->GetHeader().type == messageType);

        pool.push(message);
    }
}

std::queue<Message*>* MessageFactory::GetPoolFromType(MessageType messageType)
{
    std::queue<Message*>* resultPool = nullptr;

    std::unordered_map<MessageType, std::queue<Message*>>::iterator it = _messagePools.find(messageType);
    if (it != _messagePools.end())
    {
        resultPool = &(*it).second;
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
    case MessageType::InGame:
        resultMessage = new InGameMessage();
        break;
    case MessageType::InGameResponse:
        resultMessage = new InGameResponseMessage();
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
