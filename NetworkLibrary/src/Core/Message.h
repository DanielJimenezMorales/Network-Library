#pragma once
#include <cstdint>
#include "MessageHeader.h"

namespace NetLib
{
	class Message
	{
	public:
		MessageHeader GetHeader() const { return _header; }
		void SetHeaderPacketSequenceNumber(uint16_t packetSequenceNumber) { _header.messageSequenceNumber = packetSequenceNumber; }
		void SetReliability(bool isReliable) { _header.isReliable = isReliable; };
		void SetOrdered(bool isOrdered) { _header.isOrdered = isOrdered; }

		virtual void Write(Buffer& buffer) const = 0;
		//Read it without the message header type
		virtual void Read(Buffer& buffer) = 0;
		virtual uint32_t Size() const = 0;

		virtual ~Message() {};

	protected:
		Message(MessageType messageType) : _header(messageType, 0, false, false) {};

		MessageHeader _header;
	};

	class ConnectionRequestMessage : public Message
	{
	public:
		ConnectionRequestMessage() : clientSalt(0), Message(MessageType::ConnectionRequest) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~ConnectionRequestMessage() override {};

		uint64_t clientSalt;
	};

	class ConnectionChallengeMessage : public Message
	{
	public:
		ConnectionChallengeMessage() : clientSalt(0), serverSalt(0), Message(MessageType::ConnectionChallenge) {}

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
		ConnectionChallengeResponseMessage() : prefix(0), Message(MessageType::ConnectionChallengeResponse) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~ConnectionChallengeResponseMessage() override {};

		uint64_t prefix;
	};

	class ConnectionAcceptedMessage : public Message
	{
	public:
		ConnectionAcceptedMessage() : prefix(0), clientIndexAssigned(0), Message(MessageType::ConnectionAccepted) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~ConnectionAcceptedMessage() override {};

		uint64_t prefix;
		uint16_t clientIndexAssigned;
	};

	class ConnectionDeniedMessage : public Message
	{
	public:
		ConnectionDeniedMessage() : Message(MessageType::ConnectionDenied) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~ConnectionDeniedMessage() override {};
	};

	class DisconnectionMessage : public Message
	{
	public:
		DisconnectionMessage() : prefix(0), Message(MessageType::Disconnection) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~DisconnectionMessage() override {};

		uint64_t prefix;
	};

	class TimeRequestMessage : public Message
	{
	public:
		TimeRequestMessage() : remoteTime(0), Message(MessageType::TimeRequest) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~TimeRequestMessage() override {};

		uint32_t remoteTime;
	};

	class TimeResponseMessage : public Message
	{
	public:
		TimeResponseMessage() : remoteTime(0), serverTime(0), Message(MessageType::TimeResponse) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~TimeResponseMessage() override {};

		uint32_t remoteTime;
		uint32_t serverTime;
	};

	class InGameMessage : public Message
	{
	public:
		InGameMessage() : data(0), Message(MessageType::InGame) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~InGameMessage() override {};

		uint64_t data;
	};

	class InGameResponseMessage : public Message
	{
	public:
		InGameResponseMessage() : data(0), Message(MessageType::InGameResponse) {}

		void Write(Buffer& buffer) const override;
		void Read(Buffer& buffer) override;
		uint32_t Size() const override;

		~InGameResponseMessage() override {};

		uint64_t data;
	};
}
