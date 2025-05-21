#pragma once

namespace Engine
{
	namespace ECS
	{
		class GameEntity;
		class World;
	}
}

namespace NetLib
{
	class Buffer;
}

void SerializeForOwner( const Engine::ECS::World& world, const Engine::ECS::GameEntity& entity,
                        NetLib::Buffer& buffer );
void SerializeForNonOwner( const Engine::ECS::GameEntity& entity, NetLib::Buffer& buffer );
void DeserializeForOwner( Engine::ECS::GameEntity& entity, NetLib::Buffer& buffer );
void DeserializeForNonOwner( Engine::ECS::GameEntity& entity, NetLib::Buffer& buffer );