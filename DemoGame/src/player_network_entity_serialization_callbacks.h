#pragma once

namespace ECS
{
	class GameEntity;
}

namespace NetLib
{
	class Buffer;
}

void SerializeForOwner( const ECS::GameEntity& entity, NetLib::Buffer& buffer );
void SerializeForNonOwner( const ECS::GameEntity& entity, NetLib::Buffer& buffer );
void DeserializeForOwner( ECS::GameEntity& entity, NetLib::Buffer& buffer );