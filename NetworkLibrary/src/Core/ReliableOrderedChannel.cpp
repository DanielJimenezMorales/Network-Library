#include <sstream>

#include "ReliableOrderedChannel.h"
#include "Message.h"
#include "BitwiseUtils.h"
#include "Logger.h"
#include "MessageFactory.h"

ReliableOrderedChannel::ReliableOrderedChannel() : TransmissionChannel(TransmissionChannelType::ReliableOrdered),
_initialTimeout(0.1f),
_lastMessageSequenceNumberAcked(0),
_nextOrderedMessageSequenceNumber(0),
_reliableMessageEntriesBufferSize(1024)
{
	_reliableMessageEntries.reserve(_reliableMessageEntriesBufferSize);
	for (unsigned int i = 0; i < _reliableMessageEntriesBufferSize; ++i)
	{
		_reliableMessageEntries.push_back(ReliableMessageEntry());
	}
}

void ReliableOrderedChannel::AddMessageToSend(Message* message)
{
	_unsentMessages.push_back(message);
}

bool ReliableOrderedChannel::ArePendingMessagesToSend() const
{
	return (!_unsentMessages.empty() || AreUnackedMessagesToResend());
}

Message* ReliableOrderedChannel::GetMessageToSend()
{
	Message* message = nullptr;
	if (!_unsentMessages.empty())
	{
		message = _unsentMessages[0];
		_unsentMessages.erase(_unsentMessages.begin());
	}
	else
	{
		message = GetUnackedMessageToResend();
	}

	//TODO Check that this is not callen when message == nullptr. GetUnackedMessageToResend could return a nullptr (Although it would be an error tbh)
	AddUnackedReliableMessage(message);
	return message;
}

void ReliableOrderedChannel::AddReceivedMessage(Message* message)
{
	uint16_t messageSequenceNumber = message->GetHeader().messageSequenceNumber;
	if (IsMessageDuplicated(messageSequenceNumber))
	{
		std::stringstream ss;
		ss << "The message with ID = " << messageSequenceNumber << " is duplicated. Ignoring it...";
		LOG_INFO(ss.str());

		MessageFactory* messageFactory = MessageFactory::GetInstance();
		messageFactory->ReleaseMessage(message);
		return;
	}
	else
	{
		AckReliableMessage(messageSequenceNumber);
		if (messageSequenceNumber == _nextOrderedMessageSequenceNumber)
		{
			_readyToProcessMessages.push(message);
			++_nextOrderedMessageSequenceNumber;

			bool continueProcessing = true;
			unsigned int index = 0;
			while (continueProcessing)
			{
				if (_orderedMessagesWaitingForPrevious.empty())
				{
					continueProcessing = false;
				}
				else if (DoesUnorderedMessagesContainsSequence(_nextOrderedMessageSequenceNumber, index))
				{
					std::list<Message*>::iterator it = _orderedMessagesWaitingForPrevious.begin();
					std::advance(it, index);
					_readyToProcessMessages.push((*it));
					_orderedMessagesWaitingForPrevious.erase(it);
					++_nextOrderedMessageSequenceNumber;
				}
				else
				{
					continueProcessing = false;
				}
			}
		}
		else
		{
			AddOrderedMessage(message);
		}
	}
}

bool ReliableOrderedChannel::ArePendingReadyToProcessMessages() const
{
	return !_readyToProcessMessages.empty();
}

const Message* ReliableOrderedChannel::GetReadyToProcessMessage()
{
	if (!ArePendingReadyToProcessMessages())
	{
		return nullptr;
	}

	Message* message = nullptr;
	message = _readyToProcessMessages.front();
	_readyToProcessMessages.pop();

	_processedMessages.push(message);

	return message;
}

bool ReliableOrderedChannel::AreUnackedMessagesToResend() const
{
	std::list<float>::const_iterator cit = _unackedReliableMessageTimeouts.cbegin();
	bool found = false;
	while (cit != _unackedReliableMessageTimeouts.cend() && !found)
	{
		float timeout = *cit;
		if (timeout <= 0)
		{
			found = true;
		}

		++cit;
	}

	return found;
}

Message* ReliableOrderedChannel::GetUnackedMessageToResend()
{
	int index = GetNextUnackedMessageIndexToResend();
	if (index == -1)
	{
		return nullptr;
	}

	std::list<Message*>::iterator it = _unackedReliableMessages.begin();
	std::advance(it, index);

	Message* message = *it;
	_unackedReliableMessages.erase(it);

	std::list<float>::iterator it2 = _unackedReliableMessageTimeouts.begin();
	std::advance(it2, index);
	_unackedReliableMessageTimeouts.erase(it2);

	return message;
}

int ReliableOrderedChannel::GetNextUnackedMessageIndexToResend() const
{
	int index = 0;
	std::list<float>::const_iterator cit = _unackedReliableMessageTimeouts.cbegin();
	bool found = false;
	while (cit != _unackedReliableMessageTimeouts.cend() && !found)
	{
		float timeout = *cit;
		if (timeout <= 0)
		{
			found = true;
		}
		else
		{
			++index;
			++cit;
		}
	}

	if (!found)
	{
		index = -1;
	}

	return index;
}

void ReliableOrderedChannel::AddUnackedReliableMessage(Message* message)
{
	_unackedReliableMessages.push_back(message);
	_unackedReliableMessageTimeouts.push_back(_initialTimeout);
}

void ReliableOrderedChannel::AckReliableMessage(uint16_t messageSequenceNumber)
{
	unsigned int index = GetRollingBufferIndex(messageSequenceNumber);
	_reliableMessageEntries[index].sequenceNumber = messageSequenceNumber;
	_reliableMessageEntries[index].isAcked = true;

	_lastMessageSequenceNumberAcked = messageSequenceNumber;
}

bool ReliableOrderedChannel::DoesUnorderedMessagesContainsSequence(uint16_t sequence, unsigned int& index) const
{
	const Message* message = nullptr;
	unsigned int idx = 0;
	for (std::list<Message*>::const_iterator cit = _orderedMessagesWaitingForPrevious.cbegin(); cit != _orderedMessagesWaitingForPrevious.cend(); ++cit)
	{
		message = *cit;
		if (message->GetHeader().messageSequenceNumber == sequence)
		{
			index = idx;
			return true;
		}

		++idx;
	}

	return false;
}

bool ReliableOrderedChannel::AddOrderedMessage(Message* message)
{
	_orderedMessagesWaitingForPrevious.push_back(message);
	return true;
	//TODO Ver para qué es esto de abajo y por qué este return de aquí arriba
	std::list<Message*>::iterator it = _orderedMessagesWaitingForPrevious.begin();
	if (_orderedMessagesWaitingForPrevious.empty())
	{
		_orderedMessagesWaitingForPrevious.insert(it, message);
		return true;
	}

	bool found = false;
	while (it != _orderedMessagesWaitingForPrevious.end() && !found)
	{
		uint16_t sequenceNumber = (*it)->GetHeader().messageSequenceNumber;
		if (sequenceNumber > message->GetHeader().messageSequenceNumber)
		{
			found = true;
		}
		else
		{
			++it;
		}
	}

	_orderedMessagesWaitingForPrevious.insert(it, message);

	return found;
}

bool ReliableOrderedChannel::TryRemoveUnackedReliableMessageFromSequence(uint16_t sequence)
{
	bool result = false;

	int index = GetPendingUnackedReliableMessageIndexFromSequence(sequence);
	if (index != -1)
	{
		DeleteUnackedReliableMessageAtIndex(index);
		result = true;
	}

	return result;
}

int ReliableOrderedChannel::GetPendingUnackedReliableMessageIndexFromSequence(uint16_t sequence) const
{
	int resultIndex = -1;
	unsigned int currentIndex = 0;
	for (std::list<Message*>::const_iterator it = _unackedReliableMessages.cbegin(); it != _unackedReliableMessages.cend(); ++it)
	{
		if ((*it)->GetHeader().messageSequenceNumber == sequence)
		{
			resultIndex = currentIndex;
			break;
		}

		++currentIndex;
	}

	return resultIndex;
}

void ReliableOrderedChannel::DeleteUnackedReliableMessageAtIndex(unsigned int index)
{
	assert(index < _unackedReliableMessages.size());

	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);

	std::list<Message*>::iterator it = _unackedReliableMessages.begin();
	std::advance(it, index);
	Message* message = *it;

	_unackedReliableMessages.erase(it);

	messageFactory->ReleaseMessage(message);

	LOG_INFO("DELETE");
}

const ReliableMessageEntry& ReliableOrderedChannel::GetReliableMessageEntry(uint16_t sequenceNumber) const
{
	unsigned int index = GetRollingBufferIndex(sequenceNumber);
	return _reliableMessageEntries[index];
}

uint32_t ReliableOrderedChannel::GenerateACKs() const
{
	uint32_t acks = 0;
	uint16_t firstSequenceNumber = _lastMessageSequenceNumberAcked - 1;
	for (unsigned int i = 0; i < 32; ++i)
	{
		uint16_t currentSequenceNumber = firstSequenceNumber - i;
		const ReliableMessageEntry& reliableMessageEntry = GetReliableMessageEntry(currentSequenceNumber);
		if (reliableMessageEntry.isAcked && currentSequenceNumber == reliableMessageEntry.sequenceNumber)
		{
			BitwiseUtils::SetBitAtIndex(acks, i);
		}
	}
	return acks;
}

void ReliableOrderedChannel::ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber)
{
	std::stringstream ss;
	ss << "Last acked from client = " << lastAckedMessageSequenceNumber;
	LOG_INFO(ss.str());

	//Check if the last acked is in reliable messages lists
	TryRemoveUnackedReliableMessageFromSequence(lastAckedMessageSequenceNumber);

	//Check for the rest of acked bits
	uint16_t firstAckSequence = lastAckedMessageSequenceNumber - 1;
	for (unsigned int i = 0; i < 32; ++i)
	{
		if (BitwiseUtils::GetBitAtIndex(acks, i))
		{
			TryRemoveUnackedReliableMessageFromSequence(firstAckSequence - i);
		}
	}
}

bool ReliableOrderedChannel::IsMessageDuplicated(uint16_t messageSequenceNumber) const
{
	bool result = false;

	unsigned int index = GetRollingBufferIndex(messageSequenceNumber);
	if (_reliableMessageEntries[index].sequenceNumber == messageSequenceNumber && _reliableMessageEntries[index].isAcked)
	{
		result = true;
	}

	return result;
}

void ReliableOrderedChannel::Update(float deltaTime)
{
	//Update unacked message timeouts
	std::list<float>::iterator it = _unackedReliableMessageTimeouts.begin();
	while (it != _unackedReliableMessageTimeouts.end())
	{
		float timeout = *it;
		timeout -= deltaTime;

		if (timeout < 0)
		{
			timeout = 0;
		}

		*it = timeout;

		++it;
	}
}

ReliableOrderedChannel::~ReliableOrderedChannel()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);

	std::list<Message*>::iterator it = _unackedReliableMessages.begin();
	while (it != _unackedReliableMessages.end())
	{
		Message* message = *it;
		*it = nullptr;
		messageFactory->ReleaseMessage(message);

		++it;
	}

	_unackedReliableMessages.clear();

	it = _orderedMessagesWaitingForPrevious.begin();
	while (it != _orderedMessagesWaitingForPrevious.end())
	{
		Message* message = *it;
		*it = nullptr;
		messageFactory->ReleaseMessage(message);

		++it;
	}

	_orderedMessagesWaitingForPrevious.clear();
}

void ReliableOrderedChannel::FreeSentMessage(MessageFactory& messageFactory, Message* message)
{
	_unackedReliableMessages.push_back(message);
}
