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

ReliableOrderedChannel::ReliableOrderedChannel(ReliableOrderedChannel&& other) noexcept :
	TransmissionChannel(std::move(other)),
	_lastMessageSequenceNumberAcked(std::move(other._lastMessageSequenceNumberAcked)),		//unnecessary move, just in case I change that type
	_nextOrderedMessageSequenceNumber(std::move(other._nextOrderedMessageSequenceNumber)),	//unnecessary move, just in case I change that type
	_reliableMessageEntriesBufferSize(std::move(other._reliableMessageEntriesBufferSize)),	//unnecessary move, just in case I change that type
	_areUnsentACKs(std::move(other._areUnsentACKs)),											//unnecessary move, just in case I change that type
	_rttMilliseconds(std::move(other._rttMilliseconds)),										//unnecessary move, just in case I change that type
	_unackedReliableMessages(std::move(other._unackedReliableMessages)),
	_unackedReliableMessageTimeouts(std::move(other._unackedReliableMessageTimeouts)),
	_reliableMessageEntries(std::move(other._reliableMessageEntries)),
	_unackedMessagesSendTimes(std::move(other._unackedMessagesSendTimes)),
	_messagesRTTToProcess(std::move(other._messagesRTTToProcess)),
	_orderedMessagesWaitingForPrevious(std::move(other._orderedMessagesWaitingForPrevious))
{
}

ReliableOrderedChannel& ReliableOrderedChannel::operator=(ReliableOrderedChannel&& other) noexcept
{
	//Release old messages
	ClearMessages();

	//Move data from other to this
	_lastMessageSequenceNumberAcked = std::move(other._lastMessageSequenceNumberAcked);			//unnecessary move, just in case I change that type
	_nextOrderedMessageSequenceNumber = std::move(other._nextOrderedMessageSequenceNumber);		//unnecessary move, just in case I change that type
	_reliableMessageEntriesBufferSize = std::move(other._reliableMessageEntriesBufferSize);		//unnecessary move, just in case I change that type
	_areUnsentACKs = std::move(other._areUnsentACKs);											//unnecessary move, just in case I change that type
	_rttMilliseconds = std::move(other._rttMilliseconds);										//unnecessary move, just in case I change that type
	_unackedReliableMessages = std::move(other._unackedReliableMessages);
	_unackedReliableMessageTimeouts = std::move(other._unackedReliableMessageTimeouts);
	_reliableMessageEntries = std::move(other._reliableMessageEntries);
	_unackedMessagesSendTimes = std::move(other._unackedMessagesSendTimes);
	_messagesRTTToProcess = std::move(other._messagesRTTToProcess);
	_orderedMessagesWaitingForPrevious = std::move(other._orderedMessagesWaitingForPrevious);

	TransmissionChannel::operator=(std::move(other));
	return *this;
}

void ReliableOrderedChannel::AddMessageToSend(std::unique_ptr<Message> message)
{
	_unsentMessages.push_back(std::move(message));
}

bool ReliableOrderedChannel::ArePendingMessagesToSend() const
{
	return (!_unsentMessages.empty() || AreUnackedMessagesToResend());
}

std::unique_ptr<Message> ReliableOrderedChannel::GetMessageToSend()
{
	std::unique_ptr<Message> message = nullptr;
	if (!_unsentMessages.empty())
	{
		message = std::move(_unsentMessages[0]);
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

	return std::move(message);
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

		std::list<std::unique_ptr<Message>>::const_iterator cit = _unackedReliableMessages.cbegin();
		std::advance(cit, index);

		return (*cit)->Size();
	}
}

void ReliableOrderedChannel::AddReceivedMessage(std::unique_ptr<Message> message)
{
	uint16_t messageSequenceNumber = message->GetHeader().messageSequenceNumber;
	if (IsMessageDuplicated(messageSequenceNumber))
	{
		std::stringstream ss;
		ss << "The message with ID = " << messageSequenceNumber << " is duplicated. Ignoring it...";
		LOG_INFO(ss.str());

		MessageFactory& messageFactory = MessageFactory::GetInstance();
		messageFactory.ReleaseMessage(std::move(message));
		return;
	}
	else
	{
		LOG_INFO("New message received");
		AckReliableMessage(messageSequenceNumber);
		if (messageSequenceNumber == _nextOrderedMessageSequenceNumber)
		{
			_readyToProcessMessages.push(std::move(message));
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
					std::list<std::unique_ptr<Message>>::iterator it = _orderedMessagesWaitingForPrevious.begin();
					std::advance(it, index);

					std::unique_ptr<Message> readyToProcessMessage(std::move(*it));
					_readyToProcessMessages.push(std::move(readyToProcessMessage));
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
			AddOrderedMessage(std::move(message));
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

	std::unique_ptr<Message> message(std::move(_readyToProcessMessages.front()));
	_readyToProcessMessages.pop();

	Message* messageToReturn = message.get();
	_processedMessages.push(std::move(message));

	return messageToReturn;
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

std::unique_ptr<Message> ReliableOrderedChannel::GetUnackedMessageToResend()
{
	int index = GetNextUnackedMessageIndexToResend();
	if (index == -1)
	{
		return nullptr;
	}

	std::unique_ptr<Message> message = DeleteUnackedReliableMessageAtIndex(index);

	return std::move(message);
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

void ReliableOrderedChannel::AddUnackedReliableMessage(std::unique_ptr<Message> message)
{
	const TimeClock& timeClock = TimeClock::GetInstance();
	_unackedMessagesSendTimes[message->GetHeader().messageSequenceNumber] = timeClock.GetElapsedTimeSinceStartMilliseconds();

	_unackedReliableMessages.push_back(std::move(message));
	std::stringstream ss;
	ss << "Retransmission Timeout: " << GetRetransmissionTimeout();
	LOG_INFO(ss.str());
	_unackedReliableMessageTimeouts.push_back(GetRetransmissionTimeout());
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
	for (std::list<std::unique_ptr<Message>>::const_iterator cit = _orderedMessagesWaitingForPrevious.cbegin(); cit != _orderedMessagesWaitingForPrevious.cend(); ++cit)
	{
		message = (*cit).get();
		if (message->GetHeader().messageSequenceNumber == sequence)
		{
			index = idx;
			return true;
		}

		++idx;
	}

	return false;
}

bool ReliableOrderedChannel::AddOrderedMessage(std::unique_ptr<Message> message)
{
	_orderedMessagesWaitingForPrevious.push_back(std::move(message));
	return true;

	//TODO Ver para qué es esto de abajo y por qué este return de aquí arriba
	std::list<std::unique_ptr<Message>>::iterator it = _orderedMessagesWaitingForPrevious.begin();
	if (_orderedMessagesWaitingForPrevious.empty())
	{
		_orderedMessagesWaitingForPrevious.insert(it, std::move(message));
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

	_orderedMessagesWaitingForPrevious.insert(it, std::move(message));

	return found;
}

bool ReliableOrderedChannel::TryRemoveUnackedReliableMessageFromSequence(uint16_t sequence)
{
	bool result = false;

	int index = GetPendingUnackedReliableMessageIndexFromSequence(sequence);
	if (index != -1)
	{
		std::unique_ptr<Message> message = DeleteUnackedReliableMessageAtIndex(index);

		//Calculate RTT of acked message
		const TimeClock& timeClock = TimeClock::GetInstance();
		uint64_t currentElapsedTime = timeClock.GetElapsedTimeSinceStartMilliseconds();
		uint16_t messageRTT = currentElapsedTime - _unackedMessagesSendTimes[sequence];
		std::unordered_map<uint16_t, uint16_t>::iterator it = _unackedMessagesSendTimes.find(sequence);
		_unackedMessagesSendTimes.erase(it);
		AddMessageRTTValueToProcess(messageRTT);

		//Release acked message since we no longer need it
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		messageFactory.ReleaseMessage(std::move(message));
		result = true;
	}

	return result;
}

int ReliableOrderedChannel::GetPendingUnackedReliableMessageIndexFromSequence(uint16_t sequence) const
{
	int resultIndex = -1;
	unsigned int currentIndex = 0;
	for (std::list<std::unique_ptr<Message>>::const_iterator it = _unackedReliableMessages.cbegin(); it != _unackedReliableMessages.cend(); ++it)
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

std::unique_ptr<Message> ReliableOrderedChannel::DeleteUnackedReliableMessageAtIndex(unsigned int index)
{
	assert(index < _unackedReliableMessages.size());

	std::list<std::unique_ptr<Message>>::iterator it = _unackedReliableMessages.begin();
	std::advance(it, index);
	std::unique_ptr<Message> message(std::move(*it));

	_unackedReliableMessages.erase(it);

	std::list<float>::iterator it2 = _unackedReliableMessageTimeouts.begin();
	std::advance(it2, index);
	_unackedReliableMessageTimeouts.erase(it2);

	return std::move(message);
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
			_rttMilliseconds = AlgorithmUtils::ExponentialMovingAverage(_rttMilliseconds, messageRTTValue, 10);
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

	std::list<std::unique_ptr<Message>>::iterator it = _unackedReliableMessages.begin();
	while (it != _unackedReliableMessages.end())
	{
		std::unique_ptr<Message>message(std::move(*it));
		messageFactory.ReleaseMessage(std::move(message));

		++it;
	}

	_unackedReliableMessages.clear();

	_unackedReliableMessageTimeouts.clear();

	it = _orderedMessagesWaitingForPrevious.begin();
	while (it != _orderedMessagesWaitingForPrevious.end())
	{
		std::unique_ptr<Message>message(std::move(*it));
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

void ReliableOrderedChannel::FreeSentMessage(MessageFactory& messageFactory, std::unique_ptr<Message> message)
{
	AddUnackedReliableMessage(std::move(message));
}
