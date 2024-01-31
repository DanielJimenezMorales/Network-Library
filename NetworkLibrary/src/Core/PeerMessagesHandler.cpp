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
	if (!ArePendingACKReliableMessages())
	{
		return nullptr;
	}

	Message* message = _pendingAckReliableMessages.front();
	_pendingAckReliableMessages.erase(_pendingAckReliableMessages.begin());

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
			//TODO Release message
			MessageFactory* messageFactory = MessageFactory::GetInstance();
			messageFactory->ReleaseMessage(message);
			return false;
		}
		else
		{
			AckReliableMessage(messageSequenceNumber);
			if (messageSequenceNumber == _nextOrderedMessageSequenceNumber)
			{
				_pendingMessagesReadyToProcess.push(message);
				++_nextOrderedMessageSequenceNumber;

				bool continueProcessing = true;
				unsigned int index = 0;
				while (continueProcessing)
				{
					if (_reliableUnorderedMessages.empty())
					{
						continueProcessing = false;
					}
					else if (DoesUnorderedMessagesContainsSequence(_nextOrderedMessageSequenceNumber, index))
					{
						std::list<Message*>::iterator it = _reliableUnorderedMessages.begin();
						std::advance(it, index);
						_pendingMessagesReadyToProcess.push((*it));
						_reliableUnorderedMessages.erase(it);
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
		_pendingMessagesReadyToProcess.push(message);
	}

	return true;
}

Message* PeerMessagesHandler::GetPendingReadyToProcessMessage()
{
	if (!ArePendingReadyToProcessMessages())
	{
		return nullptr;
	}

	Message* message = nullptr;
	message = _pendingMessagesReadyToProcess.front();
	_pendingMessagesReadyToProcess.pop();

	return message;
}

bool PeerMessagesHandler::DoesUnorderedMessagesContainsSequence(uint16_t sequence, unsigned int& index) const
{
	const Message* message = nullptr;
	unsigned int idx = 0;
	for (std::list<Message*>::const_iterator cit = _reliableUnorderedMessages.cbegin(); cit != _reliableUnorderedMessages.cend(); ++cit)
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
	_reliableUnorderedMessages.push_back(message);
	return true;

	std::list<Message*>::iterator it = _reliableUnorderedMessages.begin();
	if (_reliableUnorderedMessages.empty())
	{
		_reliableUnorderedMessages.insert(it, message);
		return true;
	}

	bool found = false;
	while (it != _reliableUnorderedMessages.end() && !found)
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

	_reliableUnorderedMessages.insert(it, message);

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
	MessageFactory* messageFactory = MessageFactory::GetInstance();

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		if (message->GetHeader().isReliable)
		{
			_pendingAckReliableMessages.push_back(message);
		}
		else
		{
			messageFactory->ReleaseMessage(message);
		}
	}
}

void PeerMessagesHandler::ClearMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		messageFactory->ReleaseMessage(message);
	}

	for (unsigned int i = 0; i < _pendingMessages.size(); ++i)
	{
		Message* message = _pendingMessages[i];
		_pendingMessages[i] = nullptr;

		messageFactory->ReleaseMessage(message);
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
	TryRemovePendingACKReliableMessageFromSequence(lastAckedMessageSequenceNumber);

	//Check for the rest of acked bits
	uint16_t firstAckSequence = lastAckedMessageSequenceNumber - 1;
	for (unsigned int i = 0; i < 32; ++i)
	{
		if (BitwiseUtils::GetBitAtIndex(acks, i))
		{
			TryRemovePendingACKReliableMessageFromSequence(firstAckSequence - i);
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

int PeerMessagesHandler::GetPendingACKReliableMessageIndexFromSequence(uint16_t sequence) const
{
	int resultIndex = -1;
	unsigned int currentIndex = 0;
	for (std::list<Message*>::const_iterator it = _pendingAckReliableMessages.cbegin(); it != _pendingAckReliableMessages.cend(); ++it)
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

void PeerMessagesHandler::DeletePendingACKReliableMessageAtIndex(unsigned int index)
{
	assert(index < _pendingAckReliableMessages.size());

	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);

	std::list<Message*>::iterator it = _pendingAckReliableMessages.begin();
	std::advance(it, index);
	Message* message = *it;

	_pendingAckReliableMessages.erase(it);

	messageFactory->ReleaseMessage(message);

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

bool PeerMessagesHandler::TryRemovePendingACKReliableMessageFromSequence(uint16_t sequence)
{
	bool result = false;

	int index = GetPendingACKReliableMessageIndexFromSequence(sequence);
	if (index != -1)
	{
		DeletePendingACKReliableMessageAtIndex(index);
		result = true;
	}

	return result;
}
