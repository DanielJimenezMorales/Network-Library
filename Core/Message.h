#pragma once
#include <cstdint>
#include "MessageHeader.h"

class Message
{
public:
	Message() {};
	virtual void Write(Buffer& buffer) const = 0;
	//Read it without the message header type
	virtual void Read(Buffer& buffer) = 0;
	virtual uint32_t Size() const = 0;

	virtual ~Message() {};

	MessageHeader header;
};

class ConnectionRequestMessage : public Message
{
public:
	ConnectionRequestMessage() : clientSalt(0)
	{
		header = MessageHeader(MessageType::ConnectionRequest);
	}

	void Write(Buffer& buffer) const override;
	void Read(Buffer& buffer) override;
	uint32_t Size() const override;

	~ConnectionRequestMessage() override {};

	uint64_t clientSalt;
};

class ConnectionChallengeMessage : public Message
{
public:
	ConnectionChallengeMessage() : clientSalt(0), serverSalt(0)
	{
		header = MessageHeader(MessageType::ConnectionChallenge);
	}

	void Write(Buffer& buffer) const override;
	void Read(Buffer& buffer) override;
	uint32_t Size() const override;

	~ConnectionChallengeMessage() override {};

	uint64_t clientSalt;
	uint64_t serverSalt;
};

class ConnectionChallengeResponseMessage : public Message
{
public:
	ConnectionChallengeResponseMessage() : prefix(0)
	{
		header = MessageHeader(MessageType::ConnectionChallengeResponse);
	}

	void Write(Buffer& buffer) const override;
	void Read(Buffer& buffer) override;
	uint32_t Size() const override;

	~ConnectionChallengeResponseMessage() override {};

	uint64_t prefix;
};

class ConnectionAcceptedMessage : public Message
{
public:
	ConnectionAcceptedMessage() : prefix(0), clientIndexAssigned(0)
	{
		header = MessageHeader(MessageType::ConnectionAccepted);
	}

	void Write(Buffer & buffer) const override;
	void Read(Buffer & buffer) override;
	uint32_t Size() const override;

	~ConnectionAcceptedMessage() override {};

	uint64_t prefix;
	uint16_t clientIndexAssigned;
};

class ConnectionDeniedMessage : public Message
{
public:
	ConnectionDeniedMessage()
	{
		header = MessageHeader(MessageType::ConnectionDenied);
	}

	void Write(Buffer& buffer) const override;
	void Read(Buffer& buffer) override;
	uint32_t Size() const override;

	~ConnectionDeniedMessage() override {};
};

class DisconnectionMessage : public Message
{
public:
	DisconnectionMessage() : prefix(0)
	{
		header = MessageHeader(MessageType::Disconnection);
	}

	void Write(Buffer& buffer) const override;
	void Read(Buffer& buffer) override;
	uint32_t Size() const override;

	~DisconnectionMessage() override {};

	uint64_t prefix;
};