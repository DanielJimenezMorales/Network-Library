#pragma once
#include <cstdint>
#include <vector>

class Buffer;
class Message;

struct NetworkPacketHeader
{
	NetworkPacketHeader() : lastAckedSequenceNumber(0), ackBits(0), channelType(0) {}
	NetworkPacketHeader(uint16_t ack, uint32_t ack_bits, uint8_t channel_type) : lastAckedSequenceNumber(ack), ackBits(ack_bits), channelType(channel_type){}

	void Write(Buffer& buffer) const;
	void Read(Buffer& buffer);

	static uint32_t Size() { return sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint8_t); };

	void SetACKs(uint32_t acks) { ackBits = acks; };
	void SetHeaderLastAcked(uint16_t lastAckedMessage) { lastAckedSequenceNumber = lastAckedMessage; };
	void SetChannelType(uint8_t type) { channelType = type; };

	uint16_t lastAckedSequenceNumber;
	uint32_t ackBits;
	uint8_t channelType;
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

	uint32_t Size() const;
	unsigned int MaxSize() const { return _defaultMTUSizeInBytes; };
	bool CanMessageFit(unsigned int sizeOfMessagesInBytes) const;

	void SetHeaderACKs(uint32_t acks) { _header.SetACKs(acks); };
	void SetHeaderLastAcked(uint16_t lastAckedMessage) { _header.SetHeaderLastAcked(lastAckedMessage); };
	void SetHeaderChannelType(uint8_t channelType) { _header.SetChannelType(channelType); };

	~NetworkPacket();

private:
	const unsigned int _defaultMTUSizeInBytes;
	NetworkPacketHeader _header;
	std::vector<Message*> _messages;
};