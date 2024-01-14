#pragma once
#include <cstdint>
#include <vector>

class Buffer;
class Message;

struct NetworkPacketHeader
{
	NetworkPacketHeader() {}
	NetworkPacketHeader(uint16_t sequence, uint16_t ack, uint32_t ack_bits) : sequenceNumber(sequence), lastAckedSequenceNumber(ack), ackBits(ack_bits){}

	void Write(Buffer& buffer) const;
	void Read(Buffer& buffer);

	static uint32_t Size() { return (sizeof(uint16_t) * 2) + sizeof(uint32_t); }

	uint16_t sequenceNumber;
	uint16_t lastAckedSequenceNumber;
	uint32_t ackBits;
};

class NetworkPacket
{
public:
	NetworkPacket() {}

	void Write(Buffer& buffer) const;
	void Read(Buffer& buffer);

	bool AddMessage(Message* message);
	std::vector<Message*>::const_iterator GetMessages();
	unsigned int GetNumberOfMessages() { return messages.size(); }

	void ReleaseMessages();

	uint32_t Size() const;

private:
	NetworkPacketHeader header;
	std::vector<Message*> messages;
};