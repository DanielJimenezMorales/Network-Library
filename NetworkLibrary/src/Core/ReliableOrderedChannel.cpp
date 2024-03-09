#include <sstream>
#include <memory>

#include "ReliableOrderedChannel.h"
#include "Message.h"
#include "BitwiseUtils.h"
#include "Logger.h"
#include "MessageFactory.h"
#include "TimeClock.h"
#include "AlgorithmUtils.h"

ReliableOrderedChannel::ReliableOrderedChannel() : TransmissionChannel(TransmissionChannelType::ReliableOrdered),
_initialTimeout(0.5f),
_lastMessageSequenceNumberAcked(0),
_nextOrderedMessageSequenceNumber(0),
_reliableMessageEntriesBufferSize(1024),
_areUnsentACKs(false),
_rttMilliseconds(0)
{
	_reliableMessageEntries.reserve(_reliableMessageEntriesBufferSize);
	for (unsigned int i = 0; i < _reliableMessageEntriesBufferSize; ++i)
	{
		_reliableMessageEntries.emplace_back();
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

		uint16_t sequenceNumber = GetNextMessageSequenceNumber();
		IncreaseMessageSequenceNumber();

		message->SetHeaderPacketSequenceNumber(sequenceNumber);
	}
	else
	{
		message = GetUnackedMessageToResend();
	}

	//TODO Check that this is not called when message == nullptr. GetUnackedMessageToResend could return a nullptr (Although it would be an error tbh)
	_sentMessages.push(message);

	return message;
}

unsigned int ReliableOrderedChannel::GetSizeOfNextUnsentMessage() const
{
	if (!ArePendingMessagesToSend() && !AreUnackedMessagesToResend())
	{
		return 0;
	}

	if (!_unsentMessages.empty())
	{
		return _unsentMessages.front()->Size();
	}
	else
	{
		//Get next unacked message's size
		int index = GetNextUnackedMessageIndexToResend();

		std::list<Message*>::const_iterator cit = _unackedReliableMessages.cbegin();
		std::advance(cit, index);

		return (*cit)->Size();
	}
}

void ReliableOrderedChannel::AddReceivedMessage(Message* message)
{
	uint16_t messageSequenceNumber = message->GetHeader().messageSequenceNumber;
	if (IsMessageDuplicated(messageSequenceNumber))
	{
		std::stringstream ss;
		ss << "The message with ID = " << messageSequenceNumber << " is duplicated. Ignoring it...";
		LOG_INFO(ss.str());

		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> messageHolder(message);
		messageFactory.ReleaseMessage(std::move(messageHolder));
		return;
	}
	else
	{
		LOG_INFO("New message received");
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

	Message* message = DeleteUnackedReliableMessageAtIndex(index);

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
	std::stringstream ss;
	ss << "Retransmission Timeout: " << GetRetransmissionTimeout();
	LOG_INFO(ss.str());
	_unackedReliableMessageTimeouts.push_back(GetRetransmissionTimeout());

	const TimeClock& timeClock = TimeClock::GetInstance();
	_unackedMessagesSendTimes[message->GetHeader().messageSequenceNumber] = timeClock.GetElapsedTimeInMilliseconds();
}

void ReliableOrderedChannel::AckReliableMessage(uint16_t messageSequenceNumber)
{
	unsigned int index = GetRollingBufferIndex(messageSequenceNumber);
	_reliableMessageEntries[index].sequenceNumber = messageSequenceNumber;
	_reliableMessageEntries[index].isAcked = true;

	_lastMessageSequenceNumberAcked = messageSequenceNumber;

	//Set this flag to true so in case this peer does not have any relaible messages, force it so send a reliable packet just to notify of new acked messages from remote
	_areUnsentACKs = true;
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
		Message* message = DeleteUnackedReliableMessageAtIndex(index);

		//Calculate RTT of acked message
		const TimeClock& timeClock = TimeClock::GetInstance();
		uint64_t currentElapsedTime = timeClock.GetElapsedTimeInMilliseconds();
		uint16_t messageRTT = currentElapsedTime - _unackedMessagesSendTimes[sequence];
		std::unordered_map<uint16_t, uint16_t>::iterator it = _unackedMessagesSendTimes.find(sequence);
		_unackedMessagesSendTimes.erase(it);
		AddMessageRTTValueToProcess(messageRTT);

		//Release acked message since we no longer need it
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message>messageHolder(message);
		messageFactory.ReleaseMessage(std::move(messageHolder));
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

Message* ReliableOrderedChannel::DeleteUnackedReliableMessageAtIndex(unsigned int index)
{
	assert(index < _unackedReliableMessages.size());
	Message* message = nullptr;

	std::list<Message*>::iterator it = _unackedReliableMessages.begin();
	std::advance(it, index);
	message = *it;

	_unackedReliableMessages.erase(it);

	std::list<float>::iterator it2 = _unackedReliableMessageTimeouts.begin();
	std::advance(it2, index);
	_unackedReliableMessageTimeouts.erase(it2);

	return message;
}

const ReliableMessageEntry& ReliableOrderedChannel::GetReliableMessageEntry(uint16_t sequenceNumber) const
{
	unsigned int index = GetRollingBufferIndex(sequenceNumber);
	return _reliableMessageEntries[index];
}

void ReliableOrderedChannel::AddMessageRTTValueToProcess(uint16_t messageRTT)
{
	_messagesRTTToProcess.push(messageRTT);
}

void ReliableOrderedChannel::UpdateRTT()
{
	while (!_messagesRTTToProcess.empty())
	{
		uint16_t messageRTTValue = _messagesRTTToProcess.front();
		_messagesRTTToProcess.pop();

		if (_rttMilliseconds == 0)
		{
			_rttMilliseconds = messageRTTValue;
		}
		else
		{
			_rttMilliseconds = AlgorithmUtils::ExponentialMovingAverage(_rttMilliseconds, messageRTTValue, 20);
		}
	}

	std::stringstream ss;
	ss << "RTT: " << _rttMilliseconds;
	LOG_INFO(ss.str());
}

float ReliableOrderedChannel::GetRetransmissionTimeout() const
{
	if (_rttMilliseconds == 0)
	{
		return _initialTimeout;
	}

	return (float)_rttMilliseconds / 1000 * 2;
}

void ReliableOrderedChannel::ClearMessages()
{
	MessageFactory& messageFactory = MessageFactory::GetInstance();

	std::list<Message*>::iterator it = _unackedReliableMessages.begin();
	while (it != _unackedReliableMessages.end())
	{
		std::unique_ptr<Message>message(*it);
		*it = nullptr;
		messageFactory.ReleaseMessage(std::move(message));

		++it;
	}

	_unackedReliableMessages.clear();

	_unackedReliableMessageTimeouts.clear();

	it = _orderedMessagesWaitingForPrevious.begin();
	while (it != _orderedMessagesWaitingForPrevious.end())
	{
		std::unique_ptr<Message>message(*it);
		*it = nullptr;
		messageFactory.ReleaseMessage(std::move(message));

		++it;
	}

	_orderedMessagesWaitingForPrevious.clear();
}

void ReliableOrderedChannel::SeUnsentACKsToFalse()
{
	_areUnsentACKs = false;
}

bool ReliableOrderedChannel::AreUnsentACKs() const
{
	return _areUnsentACKs;
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

	//Update RTT
	UpdateRTT();
}

uint16_t ReliableOrderedChannel::GetLastMessageSequenceNumberAcked() const
{
	return _lastMessageSequenceNumberAcked;
}

void ReliableOrderedChannel::Reset()
{
	TransmissionChannel::Reset();
	ClearMessages();
	_lastMessageSequenceNumberAcked = 0;
	_nextOrderedMessageSequenceNumber = 0;
	_areUnsentACKs = false;

	for (unsigned int i = 0; i < _reliableMessageEntriesBufferSize; ++i)
	{
		_reliableMessageEntries[i].Reset();
	}
}

unsigned int ReliableOrderedChannel::GetRTTMilliseconds() const
{
	return _rttMilliseconds;
}

ReliableOrderedChannel::~ReliableOrderedChannel()
{
	ClearMessages();
}

void ReliableOrderedChannel::FreeSentMessage(MessageFactory& messageFactory, Message* message)
{
	AddUnackedReliableMessage(message);
}
