#include "PendingConnection.h"
#include "Message.h"
#include "MessageFactory.h"
#include "UnreliableOrderedTransmissionChannel.h"
#include "Logger.h"

namespace NetLib
{
	PendingConnection::PendingConnection() : _address(Address::GetInvalid()), _maxTimeout(0), _timeoutLeft(0), _clientSalt(0), _serverSalt(0)
	{
		_transmissionChannel = new UnreliableOrderedTransmissionChannel();
	}

	PendingConnection::PendingConnection(PendingConnection&& other) noexcept :
		_address(std::move(other._address)),
		_clientSalt(std::move(other._clientSalt)),						//unnecessary move, just in case I change that type
		_serverSalt(std::move(other._serverSalt)),						//unnecessary move, just in case I change that type
		_maxTimeout(std::move(other._maxTimeout)),						//unnecessary move, just in case I change that type
		_timeoutLeft(std::move(other._timeoutLeft)),					//unnecessary move, just in case I change that type
		_transmissionChannel(std::move(other._transmissionChannel))		//unnecessary move, just in case I change that type
	{
		other._transmissionChannel = nullptr;
	}

	PendingConnection& PendingConnection::operator=(PendingConnection&& other) noexcept
	{
		//Clear current data
		delete _transmissionChannel;
		_transmissionChannel = nullptr;

		//Assign new data
		_address = std::move(other._address);
		_clientSalt = std::move(other._clientSalt);						//unnecessary move, just in case I change that type
		_serverSalt = std::move(other._serverSalt);						//unnecessary move, just in case I change that type
		_maxTimeout = std::move(other._maxTimeout);						//unnecessary move, just in case I change that type
		_timeoutLeft = std::move(other._timeoutLeft);					//unnecessary move, just in case I change that type
		_transmissionChannel = std::move(other._transmissionChannel);	//unnecessary move, just in case I change that type
		other._transmissionChannel = nullptr;
		return *this;
	}

	void PendingConnection::Initialize(const Address& addr, float timeoutSeconds)
	{
		_address = addr;
		_maxTimeout = timeoutSeconds;
		_timeoutLeft = timeoutSeconds;
	}

	void PendingConnection::Tick(float deltaTime)
	{
		_timeoutLeft -= deltaTime;
		if (_timeoutLeft < 0.f)
		{
			_timeoutLeft = 0.f;
		}

		_transmissionChannel->Update(deltaTime);
	}

	bool PendingConnection::ArePendingMessages() const
	{
		return _transmissionChannel->ArePendingMessagesToSend();
	}

	bool PendingConnection::AddMessage(std::unique_ptr<Message> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		_transmissionChannel->AddMessageToSend(std::move(message));
		return true;
	}


	std::unique_ptr<Message> PendingConnection::GetAMessage()
	{
		return std::move(_transmissionChannel->GetMessageToSend());
	}

	void PendingConnection::AddSentMessage(std::unique_ptr<Message> message)
	{
		if (message != nullptr)
		{
			_transmissionChannel->AddSentMessage(std::move(message));
		}
	}

	void PendingConnection::FreeSentMessages()
	{
		_transmissionChannel->FreeSentMessages();
	}

	void PendingConnection::Reset()
	{
		_address = Address::GetInvalid();
		_maxTimeout = 0;
		_timeoutLeft = 0;
		_clientSalt = 0;
		_serverSalt = 0;

		_transmissionChannel->Reset();
	}

	PendingConnection::~PendingConnection()
	{
		delete _transmissionChannel;
		_transmissionChannel = nullptr;
	}
}
