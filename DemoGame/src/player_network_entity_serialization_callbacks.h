#pragma once

namespace ECS
{
	class GameEntity;
	class World;
}

namespace NetLib
{
	class Buffer;
}

void SerializeForOwner(const ECS::World& world, const ECS::GameEntity& entity, NetLib::Buffer& buffer );
void SerializeForNonOwner( const ECS::GameEntity& entity, NetLib::Buffer& buffer );
void DeserializeForOwner( ECS::GameEntity& entity, NetLib::Buffer& buffer );
void DeserializeForNonOwner( ECS::GameEntity& entity, NetLib::Buffer& buffer );