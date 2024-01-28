#include <cassert>
#include <sstream>

#include "PeerMessagesHandler.h"
#include "Message.h"
#include "MessageFactory.h"
#include "BitwiseUtils.h"
#include "Logger.h"

PeerMessagesHandler::PeerMessagesHandler() : _reliableMessageEntriesBufferSize(1024), _lastMessageSequenceNumberAcked(0)
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
	int index = -1;
	index = GetPendingACKReliableMessageIndexFromSequence(lastAckedMessageSequenceNumber);
	if (index != -1)
	{
		DeletePendingACKReliableMessageAtIndex(index);
	}

	//Check for the rest of acked bits
	uint16_t firstAckSequence = lastAckedMessageSequenceNumber - 1;
	for (unsigned int i = 0; i < 32; ++i)
	{
		if (BitwiseUtils::GetBitAtIndex(acks, i))
		{
			index = GetPendingACKReliableMessageIndexFromSequence(firstAckSequence - i);
			if (index != -1)
			{
				DeletePendingACKReliableMessageAtIndex(index);
			}
		}
	}
}

void PeerMessagesHandler::AckReliableMessage(uint16_t messageSequenceNumber)
{
	unsigned int index = messageSequenceNumber % _reliableMessageEntriesBufferSize;
	_reliableMessageEntries[index].sequenceNumber = messageSequenceNumber;
	_reliableMessageEntries[index].isAcked = true;

	_lastMessageSequenceNumberAcked = messageSequenceNumber;
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
	unsigned int index = sequenceNumber % _reliableMessageEntriesBufferSize;
	return _reliableMessageEntries[index];
}
