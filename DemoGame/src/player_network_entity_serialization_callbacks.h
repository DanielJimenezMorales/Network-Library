#pragma once

class GameEntity;

namespace NetLib
{
	class Buffer;
}

void SerializeForOwner( const GameEntity& entity, NetLib::Buffer& buffer );
void SerializeForNonOwner( const GameEntity& entity, NetLib::Buffer& buffer );
void DeserializeForOwner( GameEntity& entity, NetLib::Buffer& buffer );