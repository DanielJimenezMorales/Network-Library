#pragma once
#include "core/peer.h"

#include <SDL_image.h>

// TODO TEMP
namespace Engine
{
	class InputHandler;

	namespace ECS
	{
		class GameEntity;
		class World;
		class Prefab;
	}
}

class SceneInitializer
{
	public:
		void InitializeScene( Engine::ECS::World& scene, NetLib::PeerType networkPeerType,
		                      Engine::InputHandler& inputHandler, SDL_Renderer* renderer ) const;

		// TODO Temp method. It shouldn't go here
		void ConfigureCameraComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab ) const;
		void ConfigureHealthComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab ) const;
};
