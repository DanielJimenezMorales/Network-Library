#include <cassert>
#include <memory>

#include "TransmissionChannel.h"
#include "MessageFactory.h"

TransmissionChannel::TransmissionChannel(TransmissionChannelType type) : _type(type), _nextMessageSequenceNumber(0)
{
	_unsentMessages.reserve(5);
}

void TransmissionChannel::FreeSentMessages()
{
	MessageFactory& messageFactory = MessageFactory::GetInstance();

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		FreeSentMessage(messageFactory, message);
	}
}

void TransmissionChannel::FreeProcessedMessages()
{
	MessageFactory& messageFactory = MessageFactory::GetInstance();

	while (!_processedMessages.empty())
	{
		std::unique_ptr<Message> message(_processedMessages.front());
		_processedMessages.pop();

		messageFactory.ReleaseMessage(std::move(message));
	}
}

void TransmissionChannel::Reset()
{
	ClearMessages();
	_nextMessageSequenceNumber = 0;
}

TransmissionChannel::~TransmissionChannel()
{
	ClearMessages();
}

void TransmissionChannel::ClearMessages()
{
	MessageFactory& messageFactory = MessageFactory::GetInstance();

	while (!_sentMessages.empty())
	{
		std::unique_ptr<Message> message(_sentMessages.front());
		_sentMessages.pop();

		messageFactory.ReleaseMessage(std::move(message));
	}

	while (!_processedMessages.empty())
	{
		std::unique_ptr<Message> message(_processedMessages.front());
		_processedMessages.pop();

		messageFactory.ReleaseMessage(std::move(message));
	}
	
	for (std::vector<Message*>::iterator it = _unsentMessages.begin(); it != _unsentMessages.end(); ++it)
	{
		std::unique_ptr<Message> message(*it);
		messageFactory.ReleaseMessage(std::move(message));
		*it = nullptr;
	}

	_unsentMessages.clear();
}
