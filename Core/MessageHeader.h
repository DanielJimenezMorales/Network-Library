#pragma once
#include <cstdint>

class Buffer;

enum MessageType : uint8_t
{
	ConnectionRequest = 0,
	ConnectionAccepted = 1,
	ConnectionDenied = 2,
	ConnectionChallenge = 3,
	ConnectionChallengeResponse = 4,
	Disconnection = 5,
};

struct MessageHeader
{
	MessageHeader() {}
	MessageHeader(MessageType messageType) : type(messageType) {}

	void Write(Buffer& buffer) const;
	void Read(Buffer& buffer);

	~MessageHeader() {}

	MessageType type;
};
