#pragma once
#include <cstdint>
#include "Buffer.h"
#include "../Utils/BufferUtils.h"


enum NetworkPacketType : uint8_t
{
	ConnectionRequest = 0,
	ConnectionAccepted = 1,
	ConnectionDenied = 2,
	ConnectionChallenge = 3,
	ConnectionChallengeResponse = 4,
	Disconnection = 5,
};

struct NetworkConnectionRequestPacket
{
	NetworkPacketType type = NetworkPacketType::ConnectionRequest;
	uint64_t clientSalt;

	void Write(Buffer& buffer) const;
};

struct NetworkConnectionChallengePacket
{
	NetworkPacketType type = NetworkPacketType::ConnectionChallenge;
	uint64_t clientSalt;
	uint64_t serverSalt;

	void Write(Buffer& buffer) const;
};

struct NetworkConnectionChallengeResponsePacket
{
	NetworkPacketType type = NetworkPacketType::ConnectionChallengeResponse;
	uint64_t prefix;

	void Write(Buffer& buffer) const;
};

struct NetworkConnectionAcceptedPacket
{
	NetworkPacketType type = NetworkPacketType::ConnectionAccepted;
	uint64_t prefix;
	uint16_t clientIndexAssigned;

	void Write(Buffer& buffer) const;
};

struct NetworkConnectionDeniedPacket
{
	NetworkPacketType type = NetworkPacketType::ConnectionDenied;

	void Write(Buffer& buffer) const;
};

struct NetworkDisconnectionPacket
{
	NetworkPacketType type = NetworkPacketType::Disconnection;
	uint64_t prefix;

	void Write(Buffer& buffer) const;
};