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
	InGame = 6,
	InGameResponse = 7,
	TimeRequest = 8,
	TimeResponse = 9
};

struct MessageHeader
{
	MessageHeader(MessageType messageType, uint16_t packetSequenceNumber, bool isReliable) : type(messageType), messageSequenceNumber(packetSequenceNumber), isReliable(isReliable) {}

	MessageHeader(const MessageHeader& other) : type(other.type), messageSequenceNumber(other.messageSequenceNumber), isReliable(other.isReliable) {}

	void Write(Buffer& buffer) const;
	void Read(Buffer& buffer);
	void ReadWithoutHeader(Buffer& buffer);
	static uint32_t Size() { return sizeof(MessageType) + sizeof(uint16_t) + sizeof(uint8_t); }

	~MessageHeader() {}

	MessageType type;
	uint16_t messageSequenceNumber;
	bool isReliable;
};
