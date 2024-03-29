#include <sstream>
#include <memory>

#include "Client.h"
#include "Message.h"
#include "Logger.h"
#include "NetworkPacket.h"
#include "MessageFactory.h"
#include "RemotePeer.h"
#include "PendingConnection.h"
#include "TimeClock.h"

namespace NetLib
{
	Client::Client(float serverMaxInactivityTimeout) : Peer(PeerType::ClientMode, 1, 1024, 1024),
		_serverMaxInactivityTimeout(serverMaxInactivityTimeout),
		_serverInactivityTimeLeft(serverMaxInactivityTimeout),
		_saltNumber(0),
		_dataPrefix(0),
		_serverAddress("127.0.0.1", 54000),
		inGameMessageID(0),
		_timeSinceLastTimeRequest(0.0f),
		_numberOfInitialTimeRequestBurstLeft(NUMBER_OF_INITIAL_TIME_REQUESTS_BURST)
	{
	}

	Client::~Client()
	{
	}

	bool Client::StartConcrete()
	{
		sockaddr_in addressInfo;
		addressInfo.sin_family = AF_INET;
		addressInfo.sin_port = 0; //This is zero so the system picks up a random port number

		char ip[] = "127.0.0.1";
		int iResult = inet_pton(addressInfo.sin_family, ip, &addressInfo.sin_addr);
		if (iResult == -1)
		{
			std::stringstream ss;
			ss << "Error at converting IP string into address. Error code: " << WSAGetLastError();
			Common::LOG_ERROR(ss.str());
		}
		else if (iResult == 0)
		{
			std::stringstream ss;
			ss << "The IP string: " << ip << " is not valid";
			Common::LOG_ERROR(ss.str());
		}

		Address address = Address(addressInfo);
		BindSocket(address);

		_currentState = ClientState::SendingConnectionRequest;
		_serverInactivityTimeLeft = _serverMaxInactivityTimeout;

		GenerateClientSaltNumber();
		_pendingConnections.emplace_back(_serverAddress);

		Common::LOG_INFO("Client started succesfully!");

		return true;
	}

	void Client::GenerateClientSaltNumber()
	{
		//TODO Change this for a better generator. rand is not generating a full 64bit integer since its maximum is roughly 32767. I have tried to use mt19937_64 but I think I get a conflict with winsocks and std::uniform_int_distribution
		srand(time(NULL));
		_saltNumber = rand();
	}

	void Client::ProcessMessage(const Message& message, const Address& address)
	{
		MessageType messageType = message.GetHeader().type;

		switch (messageType)
		{
		case MessageType::ConnectionChallenge:
			if (_currentState == ClientState::SendingConnectionRequest || _currentState == ClientState::SendingConnectionChallengeResponse)
			{
				const ConnectionChallengeMessage& connectionChallengeMessage = static_cast<const ConnectionChallengeMessage&>(message);
				ProcessConnectionChallenge(connectionChallengeMessage);
			}
			break;
		case MessageType::ConnectionAccepted:
			if (_currentState == ClientState::SendingConnectionChallengeResponse)
			{
				const ConnectionAcceptedMessage& connectionAcceptedMessage = static_cast<const ConnectionAcceptedMessage&>(message);
				ProcessConnectionRequestAccepted(connectionAcceptedMessage);
			}
			break;
		case MessageType::ConnectionDenied:
			if (_currentState == ClientState::SendingConnectionChallengeResponse || _currentState == ClientState::SendingConnectionRequest)
			{
				const ConnectionDeniedMessage& connectionDeniedMessage = static_cast<const ConnectionDeniedMessage&>(message);
				ProcessConnectionRequestDenied(connectionDeniedMessage);
			}
			break;
		case MessageType::Disconnection:
			if (_currentState == ClientState::Connected)
			{
				const DisconnectionMessage& disconnectionMessage = static_cast<const DisconnectionMessage&>(message);
				ProcessDisconnection(disconnectionMessage);
			}
			break;
		case MessageType::TimeResponse:
			if (_currentState == ClientState::Connected)
			{
				const TimeResponseMessage& timeResponseMessage = static_cast<const TimeResponseMessage&>(message);
				ProcessTimeResponse(timeResponseMessage);
			}
			break;
		case MessageType::InGameResponse:
			if (_currentState == ClientState::Connected)
			{
				const InGameResponseMessage& inGameResponseMessage = static_cast<const InGameResponseMessage&>(message);
				ProcessInGameResponse(inGameResponseMessage);
			}
			break;
		default:
			Common::LOG_WARNING("Invalid Message type, ignoring it...");
			break;
		}
	}

	void Client::TickConcrete(float elapsedTime)
	{
		if (_currentState == ClientState::SendingConnectionRequest || _currentState == ClientState::SendingConnectionChallengeResponse)
		{
			CreateConnectionRequestMessage();
		}

		if (_currentState != ClientState::Disconnected)
		{
			_serverInactivityTimeLeft -= elapsedTime;

			if (_serverInactivityTimeLeft <= 0.f)
			{
				Common::LOG_INFO("Server inactivity timeout reached. Disconnecting client...");
				_serverInactivityTimeLeft = 0.f;
				_currentState = ClientState::Disconnected;
			}
		}

		if (_currentState == ClientState::Connected)
		{
			UpdateTimeRequestsElapsedTime(elapsedTime);
			CreateInGameMessage();
		}
	}

	void Client::DisconnectRemotePeerConcrete(RemotePeer& remotePeer)
	{
	}

	bool Client::StopConcrete()
	{
		return true;
	}

	void Client::ProcessConnectionChallenge(const ConnectionChallengeMessage& message)
	{
		Common::LOG_INFO("Challenge packet received from server");

		uint64_t clientSalt = message.clientSalt;
		uint64_t serverSalt = message.serverSalt;
		if (_saltNumber != clientSalt)
		{
			Common::LOG_WARNING("The generated salt number does not match the server's challenge client salt number. Aborting operation");
			return;
		}

		_dataPrefix = clientSalt ^ serverSalt; //XOR operation to create the data prefix for all packects from now on

		_currentState = ClientState::SendingConnectionChallengeResponse;

		CreateConnectionChallengeResponse();

		Common::LOG_INFO("Sending challenge response packet to server...");
	}

	void Client::ProcessConnectionRequestAccepted(const ConnectionAcceptedMessage& message)
	{
		uint64_t dataPrefix = message.prefix;
		if (dataPrefix != _dataPrefix)
		{
			Common::LOG_WARNING("Packet prefix does not match. Skipping packet...");
			return;
		}

		_pendingConnections.erase(_pendingConnections.begin());

		_remotePeerSlots[0] = true;
		_remotePeers[0].Connect(_serverAddress.GetInfo(), 0, 5, dataPrefix);

		_clientIndex = message.clientIndexAssigned;
		_currentState = ClientState::Connected;

		Common::LOG_INFO("Connection accepted!");
		ExecuteOnPeerConnected();
	}

	void Client::ProcessConnectionRequestDenied(const ConnectionDeniedMessage& message)
	{
		_currentState = ClientState::Disconnected;
		Common::LOG_INFO("Connection denied");
	}

	void Client::ProcessDisconnection(const DisconnectionMessage& message)
	{
		uint64_t dataPrefix = message.prefix;
		if (dataPrefix != _dataPrefix)
		{
			Common::LOG_WARNING("Packet prefix does not match. Skipping packet...");
			return;
		}

		_currentState = ClientState::Disconnected();
		Common::LOG_INFO("Disconnection message received from server. Disconnecting...");

		ExecuteOnPeerDisconnected();
	}

	void Client::ProcessTimeResponse(const TimeResponseMessage& message)
	{
		Common::LOG_INFO("PROCESSING TIME RESPONSE");

		//Add new RTT to buffer
		TimeClock& timeClock = TimeClock::GetInstance();
		unsigned int rtt = timeClock.GetLocalTimeMilliseconds() - message.remoteTime;
		_timeRequestRTTs.push_back(rtt);

		if (_timeRequestRTTs.size() == TIME_REQUEST_RTT_BUFFER_SIZE + 1)
		{
			_timeRequestRTTs.pop_front();
		}

		//Get RTT to adjust server's clock elapsed time
		unsigned int meanRTT = 0;
		if (_timeRequestRTTs.size() == TIME_REQUEST_RTT_BUFFER_SIZE)
		{
			//Sort RTTs and remove the smallest and biggest values (They are considered outliers!)
			std::list<unsigned int> sortedTimeRequestRTTs = _timeRequestRTTs;
			sortedTimeRequestRTTs.sort();

			//Remove potential outliers
			for (unsigned int i = 0; i < NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE; ++i)
			{
				sortedTimeRequestRTTs.pop_back();
				sortedTimeRequestRTTs.pop_front();
			}

			std::list<unsigned int>::const_iterator cit = sortedTimeRequestRTTs.cbegin();
			for (; cit != sortedTimeRequestRTTs.cend(); ++cit)
			{
				meanRTT += *cit;
			}

			const unsigned int NUMBER_OF_VALID_RTT_TO_AVERAGE = TIME_REQUEST_RTT_BUFFER_SIZE - (2 * NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE);

			meanRTT /= NUMBER_OF_VALID_RTT_TO_AVERAGE;
		}
		else
		{
			meanRTT = rtt;
		}

		//Calculate server clock delta time
		unsigned int serverClockElapsedTimeMilliseconds = message.serverTime - message.remoteTime - (meanRTT / 2);
		double serverClockElapsedTimeSeconds = static_cast<double>(serverClockElapsedTimeMilliseconds) / 1000;
		timeClock.SetServerClockTimeDelta(serverClockElapsedTimeSeconds);

		std::stringstream ss;
		ss << "SERVER TIME UPDATED. Local time: " << timeClock.GetLocalTimeSeconds() << "s, Server time: " << timeClock.GetServerTimeSeconds() << "s";
		Common::LOG_INFO(ss.str());
	}

	void Client::ProcessInGameResponse(const InGameResponseMessage& message)
	{
		std::stringstream ss;
		ss << "In game response ID = " << message.data;
		Common::LOG_INFO(ss.str());
	}

	void Client::CreateConnectionRequestMessage()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionRequest);

		if (message == nullptr)
		{
			Common::LOG_ERROR("Can't create new Connection Request Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<ConnectionRequestMessage> connectionRequestMessage(static_cast<ConnectionRequestMessage*>(message.release()));

		connectionRequestMessage->clientSalt = _saltNumber;

		_pendingConnections[0].AddMessage(std::move(connectionRequestMessage));

		Common::LOG_INFO("Connection request created.");
	}

	void Client::CreateConnectionChallengeResponse()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionChallengeResponse);
		if (message == nullptr)
		{
			Common::LOG_ERROR("Can't create new Connection Challenge Response Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<ConnectionChallengeResponseMessage> connectionChallengeResponseMessage(static_cast<ConnectionChallengeResponseMessage*>(message.release()));
		connectionChallengeResponseMessage->prefix = _dataPrefix;

		_pendingConnections[0].AddMessage(std::move(connectionChallengeResponseMessage));
	}

	void Client::CreateTimeRequestMessage()
	{
		Common::LOG_INFO("TIME REQUEST CREATED");
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> lendMessage(messageFactory.LendMessage(MessageType::TimeRequest));

		std::unique_ptr<TimeRequestMessage> timeRequestMessage(static_cast<TimeRequestMessage*>(lendMessage.release()));

		timeRequestMessage->SetOrdered(true);
		TimeClock& timeClock = TimeClock::GetInstance();
		timeRequestMessage->remoteTime = timeClock.GetLocalTimeMilliseconds();

		_remotePeers[0].AddMessage(std::move(timeRequestMessage));
	}

	void Client::CreateInGameMessage()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::InGame);
		if (message == nullptr)
		{
			Common::LOG_ERROR("Can't create new Connection Challenge Response Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<InGameMessage> inGameMessage(static_cast<InGameMessage*>(message.release()));
		inGameMessage->SetOrdered(true);
		inGameMessage->data = inGameMessageID;
		inGameMessageID++;
		_remotePeers[0].AddMessage(std::move(inGameMessage));
	}

	void Client::UpdateTimeRequestsElapsedTime(float elapsedTime)
	{
		if (_numberOfInitialTimeRequestBurstLeft > 0)
		{
			--_numberOfInitialTimeRequestBurstLeft;
			CreateTimeRequestMessage();
			return;
		}

		_timeSinceLastTimeRequest += elapsedTime;
		if (_timeSinceLastTimeRequest >= TIME_REQUESTS_FREQUENCY_SECONDS)
		{
			_timeSinceLastTimeRequest = 0;
			CreateTimeRequestMessage();
		}
	}
}
