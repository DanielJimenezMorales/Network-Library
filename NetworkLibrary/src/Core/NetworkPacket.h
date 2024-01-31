#pragma once
#include <cstdint>
#include <vector>

class Buffer;
class Message;

struct NetworkPacketHeader
{
	NetworkPacketHeader() : lastAckedSequenceNumber(0), ackBits(0) {}
	NetworkPacketHeader(uint16_t ack, uint32_t ack_bits) : lastAckedSequenceNumber(ack), ackBits(ack_bits){}

	void Write(Buffer& buffer) const;
	void Read(Buffer& buffer);

	static uint32_t Size() { return sizeof(uint16_t) + sizeof(uint32_t); };

	void SetACKs(uint32_t acks) { ackBits = acks; };
	void SetHeaderLastAcked(uint16_t lastAckedMessage) { lastAckedSequenceNumber = lastAckedMessage; };

	uint16_t lastAckedSequenceNumber;
	uint32_t ackBits;
};

class NetworkPacket
{
public:
	//NetworkPacket() : _defaultMTUSizeInBytes(1500) {};
	NetworkPacket();

	void Write(Buffer& buffer) const;
	void Read(Buffer& buffer);

	const NetworkPacketHeader& GetHeader() const { return _header; };

	bool AddMessage(Message* message);
	std::vector<Message*>::iterator GetMessages();
	unsigned int GetNumberOfMessages() const { return _messages.size(); }

	void ReleaseMessages();

	uint32_t Size() const;
	unsigned int MaxSize() const { return _defaultMTUSizeInBytes; };
	bool CanMessageFit(unsigned int sizeOfMessagesInBytes) const;

	void SetHeaderACKs(uint32_t acks) { _header.SetACKs(acks); };
	void SetHeaderLastAcked(uint16_t lastAckedMessage) { _header.SetHeaderLastAcked(lastAckedMessage); };

private:
	const unsigned int _defaultMTUSizeInBytes;
	NetworkPacketHeader _header;
	std::vector<Message*> _messages;
};