#include <cassert>

#include "TransmissionChannel.h"
#include "MessageFactory.h"

TransmissionChannel::TransmissionChannel(TransmissionChannelType type) : _type(type)
{
	_unsentMessages.reserve(5);
}

void TransmissionChannel::FreeSentMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		FreeSentMessage(*messageFactory, message);
	}
}

void TransmissionChannel::FreeProcessedMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);

	while (!_processedMessages.empty())
	{
		Message* message = _processedMessages.front();
		_processedMessages.pop();

		messageFactory->ReleaseMessage(message);
	}
}

TransmissionChannel::~TransmissionChannel()
{
	ClearMessages();
}

void TransmissionChannel::ClearMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		messageFactory->ReleaseMessage(message);
	}

	while (!_processedMessages.empty())
	{
		Message* message = _processedMessages.front();
		_processedMessages.pop();

		messageFactory->ReleaseMessage(message);
	}
	
	for (std::vector<Message*>::iterator it = _unsentMessages.begin(); it != _unsentMessages.end(); ++it)
	{
		Message* message = *it;
		messageFactory->ReleaseMessage(message);
		*it = nullptr;
	}

	_unsentMessages.clear();
}