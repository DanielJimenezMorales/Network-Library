#include <cassert>
#include <sstream>

#include "PeerMessagesHandler.h"
#include "Message.h"
#include "MessageFactory.h"
#include "BitwiseUtils.h"
#include "Logger.h"
#include "MessageFactory.h"

PeerMessagesHandler::PeerMessagesHandler() : _reliableMessageEntriesBufferSize(1024), _lastMessageSequenceNumberAcked(0), _nextOrderedMessageSequenceNumber(0)
{
	_pendingMessages.reserve(5);
	_reliableMessageEntries.reserve(_reliableMessageEntriesBufferSize);
	for (unsigned int i = 0; i < _reliableMessageEntriesBufferSize; ++i)
	{
		_reliableMessageEntries.push_back(ReliableMessageEntry());
	}
}

bool PeerMessagesHandler::AddMessage(Message* message)
{
	_pendingMessages.push_back(message);
	return true;
}

Message* PeerMessagesHandler::GetPendingMessage()
{
	if (!ArePendingMessages())
	{
		return nullptr;
	}

	Message* message = _pendingMessages[0];
	_pendingMessages.erase(_pendingMessages.begin());

	_sentMessages.push(message);
	return message;
}

Message* PeerMessagesHandler::GetPendingACKReliableMessage()
{
	if (!AreUnackedReliableMessages())
	{
		return nullptr;
	}

	Message* message = _unackedReliableMessages.front();
	_unackedReliableMessages.erase(_unackedReliableMessages.begin());

	_sentMessages.push(message);
	return message;
}

bool PeerMessagesHandler::AddReceivedMessage(Message* message)
{
	if (message == nullptr)
	{
		return false;
	}

	MessageHeader header = message->GetHeader();
	if (header.isReliable)
	{
		uint16_t messageSequenceNumber = header.messageSequenceNumber;
		if (IsMessageDuplicated(messageSequenceNumber))
		{
			std::stringstream ss;
			ss << "The message with ID = " << messageSequenceNumber << " is duplicated. Ignoring it...";
			LOG_INFO(ss.str());

			MessageFactory& messageFactory = MessageFactory::GetInstance();
			messageFactory.ReleaseMessage(message);
			return false;
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
	else
	{
		_readyToProcessMessages.push(message);
	}

	return true;
}

const Message* PeerMessagesHandler::GetReadyToProcessMessage()
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

bool PeerMessagesHandler::DoesUnorderedMessagesContainsSequence(uint16_t sequence, unsigned int& index) const
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

bool PeerMessagesHandler::AddOrderedMessage(Message* message)
{
	_orderedMessagesWaitingForPrevious.push_back(message);
	return true;

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

unsigned int PeerMessagesHandler::GetSizeOfNextPendingMessage() const
{
	if (!ArePendingMessages())
	{
		return 0;
	}

	return _pendingMessages.front()->Size();
}

void PeerMessagesHandler::FreeSentMessages()
{
	MessageFactory& messageFactory = MessageFactory::GetInstance();

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		if (message->GetHeader().isReliable)
		{
			_unackedReliableMessages.push_back(message);
		}
		else
		{
			messageFactory.ReleaseMessage(message);
		}
	}
}

void PeerMessagesHandler::FreeProcessedMessages()
{
	MessageFactory& messageFactory = MessageFactory::GetInstance();

	while (!_processedMessages.empty())
	{
		Message* message = _processedMessages.front();
		_processedMessages.pop();

		messageFactory.ReleaseMessage(message);
	}
}

void PeerMessagesHandler::ClearMessages()
{
	MessageFactory& messageFactory = MessageFactory::GetInstance();

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		messageFactory.ReleaseMessage(message);
	}

	while (!_processedMessages.empty())
	{
		Message* message = _processedMessages.front();
		_processedMessages.pop();

		messageFactory.ReleaseMessage(message);
	}

	for (unsigned int i = 0; i < _pendingMessages.size(); ++i)
	{
		Message* message = _pendingMessages[i];
		_pendingMessages[i] = nullptr;

		messageFactory.ReleaseMessage(message);
	}

	_pendingMessages.clear();
}

uint32_t PeerMessagesHandler::GenerateACKs() const
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

void PeerMessagesHandler::ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber)
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

void PeerMessagesHandler::AckReliableMessage(uint16_t messageSequenceNumber)
{
	unsigned int index = GetRollingBufferIndex(messageSequenceNumber);
	_reliableMessageEntries[index].sequenceNumber = messageSequenceNumber;
	_reliableMessageEntries[index].isAcked = true;

	_lastMessageSequenceNumberAcked = messageSequenceNumber;
}

bool PeerMessagesHandler::IsMessageDuplicated(uint16_t messageSequenceNumber) const
{
	bool result = false;

	unsigned int index = GetRollingBufferIndex(messageSequenceNumber);
	if (_reliableMessageEntries[index].sequenceNumber == messageSequenceNumber && _reliableMessageEntries[index].isAcked)
	{
		result = true;
	}

	return result;
}

int PeerMessagesHandler::GetPendingUnackedReliableMessageIndexFromSequence(uint16_t sequence) const
{
	int resultIndex = -1;
	unsigned int currentIndex = 0;
	for (std::list<Message*>::const_iterator it = _unackedReliableMessages.cbegin(); it != _unackedReliableMessages.cend(); ++it)
	{
		if((*it)->GetHeader().messageSequenceNumber == sequence)
		{
			resultIndex = currentIndex;
			break;
		}

		++currentIndex;
	}

	return resultIndex;
}

void PeerMessagesHandler::DeleteUnackedReliableMessageAtIndex(unsigned int index)
{
	assert(index < _unackedReliableMessages.size());

	MessageFactory& messageFactory = MessageFactory::GetInstance();

	std::list<Message*>::iterator it = _unackedReliableMessages.begin();
	std::advance(it, index);
	Message* message = *it;

	_unackedReliableMessages.erase(it);

	messageFactory.ReleaseMessage(message);

	LOG_INFO("DELETE");
}

PeerMessagesHandler::~PeerMessagesHandler()
{
	ClearMessages();
}

const ReliableMessageEntry& PeerMessagesHandler::GetReliableMessageEntry(uint16_t sequenceNumber) const
{
	unsigned int index = GetRollingBufferIndex(sequenceNumber);
	return _reliableMessageEntries[index];
}

bool PeerMessagesHandler::TryRemoveUnackedReliableMessageFromSequence(uint16_t sequence)
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
