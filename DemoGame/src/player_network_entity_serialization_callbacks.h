#pragma once

class GameEntity;

namespace NetLib
{
	class Buffer;
}

static void SerializeForOwner( const GameEntity& entity, NetLib::Buffer& buffer );
static void SerializeForNonOwner( const GameEntity& entity, NetLib::Buffer& buffer );
static void DeserializeForOwner( GameEntity& entity, NetLib::Buffer& buffer );