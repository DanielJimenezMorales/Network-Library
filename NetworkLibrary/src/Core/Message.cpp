#include "Message.h"
#include "Buffer.h"
#include "Logger.h"

namespace NetLib
{
	void ConnectionRequestMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteLong(clientSalt);
	}

	void ConnectionRequestMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::ConnectionRequest;
		_header.ReadWithoutHeader(buffer);

		clientSalt = buffer.ReadLong();
	}

	uint32_t ConnectionRequestMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint64_t);
	}

	void ConnectionChallengeMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteLong(clientSalt);
		buffer.WriteLong(serverSalt);
	}

	void ConnectionChallengeMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::ConnectionChallenge;
		_header.ReadWithoutHeader(buffer);

		clientSalt = buffer.ReadLong();
		serverSalt = buffer.ReadLong();
	}

	uint32_t ConnectionChallengeMessage::Size() const
	{
		return MessageHeader::Size() + (sizeof(uint64_t) * 2);
	}

	void ConnectionChallengeResponseMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteLong(prefix);
	}

	void ConnectionChallengeResponseMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::ConnectionChallengeResponse;
		_header.ReadWithoutHeader(buffer);

		prefix = buffer.ReadLong();
	}

	uint32_t ConnectionChallengeResponseMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint64_t);
	}

	void ConnectionAcceptedMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteLong(prefix);
		buffer.WriteShort(clientIndexAssigned);
	}

	void ConnectionAcceptedMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::ConnectionAccepted;
		_header.ReadWithoutHeader(buffer);

		prefix = buffer.ReadLong();
		clientIndexAssigned = buffer.ReadShort();
	}

	uint32_t ConnectionAcceptedMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint64_t) + sizeof(uint16_t);
	}

	void ConnectionDeniedMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteByte(reason);
	}

	void ConnectionDeniedMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::ConnectionDenied;
		_header.ReadWithoutHeader(buffer);
		reason = buffer.ReadByte();
	}

	uint32_t ConnectionDeniedMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint8_t);
	}

	void DisconnectionMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteLong(prefix);
		buffer.WriteByte(reason);
	}

	void DisconnectionMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::Disconnection;
		_header.ReadWithoutHeader(buffer);

		prefix = buffer.ReadLong();
		reason = buffer.ReadByte();
	}

	uint32_t DisconnectionMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint64_t) + sizeof(uint8_t);
	}

	void TimeRequestMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteInteger(remoteTime);
	}

	void TimeRequestMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::TimeRequest;
		_header.ReadWithoutHeader(buffer);

		remoteTime = buffer.ReadInteger();
	}

	uint32_t TimeRequestMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint32_t);
	}

	void TimeResponseMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteInteger(remoteTime);
		buffer.WriteInteger(serverTime);
	}

	void TimeResponseMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::TimeResponse;
		_header.ReadWithoutHeader(buffer);

		remoteTime = buffer.ReadInteger();
		serverTime = buffer.ReadInteger();
	}

	uint32_t TimeResponseMessage::Size() const
	{
		return MessageHeader::Size() + (2 * sizeof(uint32_t));
	}

	void InGameMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteLong(data);
	}

	void InGameMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::InGame;
		_header.ReadWithoutHeader(buffer);

		data = buffer.ReadLong();
	}

	uint32_t InGameMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint64_t);
	}

	void InGameResponseMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteLong(data);
	}

	void InGameResponseMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::InGameResponse;
		_header.ReadWithoutHeader(buffer);

		data = buffer.ReadLong();
	}

	uint32_t InGameResponseMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint64_t);
	}

	void ReplicationMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);
		buffer.WriteByte(replicationAction);
		buffer.WriteInteger(networkEntityId);
		buffer.WriteInteger(controlledByPeerId);
		buffer.WriteInteger(replicatedClassId);
		buffer.WriteShort(dataSize);
		//TODO Create method called WriteData(data, size) in order to avoid this for loop
		for (size_t i = 0; i < dataSize; ++i)
		{
			buffer.WriteByte(data[i]);
		}
	}

	void ReplicationMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::Replication;
		_header.ReadWithoutHeader(buffer);

		replicationAction = buffer.ReadByte();
		networkEntityId = buffer.ReadInteger();
		controlledByPeerId = buffer.ReadInteger();
		replicatedClassId = buffer.ReadInteger();
		dataSize = buffer.ReadShort();
		if (dataSize > 0)
		{
			data = new uint8_t[dataSize];
		}

		//TODO Create method called ReadData(uint8_t& data, size) in order to avoid this for loop
		for (size_t i = 0; i < dataSize; ++i)
		{
			data[i] = buffer.ReadByte();
		}
	}

	uint32_t ReplicationMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint8_t) + (3 * sizeof(uint32_t)) + sizeof(uint16_t) + (dataSize * sizeof(uint8_t));
	}

	void ReplicationMessage::Reset()
	{
		if (data != nullptr)
		{
			delete[] data;
			data = nullptr;
		}
	}

	ReplicationMessage::~ReplicationMessage()
	{
		if (data != nullptr)
		{
			delete[] data;
			data = nullptr;
		}
	}

	void InputStateMessage::Write(Buffer& buffer) const
	{
		_header.Write(buffer);

		buffer.WriteShort(dataSize);
		//TODO Create method called WriteData(data, size) in order to avoid this for loop
		for (size_t i = 0; i < dataSize; ++i)
		{
			buffer.WriteByte(data[i]);
		}
	}

	void InputStateMessage::Read(Buffer& buffer)
	{
		_header.type = MessageType::Inputs;
		_header.ReadWithoutHeader(buffer);

		dataSize = buffer.ReadShort();
		if (dataSize > 0)
		{
			data = new uint8_t[dataSize];
		}

		//TODO Create method called ReadData(uint8_t& data, size) in order to avoid this for loop
		for (size_t i = 0; i < dataSize; ++i)
		{
			data[i] = buffer.ReadByte();
		}
	}

	uint32_t InputStateMessage::Size() const
	{
		return MessageHeader::Size() + sizeof(uint16_t) + (dataSize * sizeof(uint8_t));
	}

	void InputStateMessage::Reset()
	{
		if (data != nullptr)
		{
			delete[] data;
			data = nullptr;
		}
	}
}
