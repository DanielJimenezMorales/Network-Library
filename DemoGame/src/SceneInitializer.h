#pragma once
#include "core/peer.h"

#include <SDL_image.h>

class InputHandler;

// TODO TEMP
namespace ECS
{
	class GameEntity;
	class World;
	class Prefab;
}

class SceneInitializer
{
	public:
		void InitializeScene( ECS::World& scene, NetLib::PeerType networkPeerType, InputHandler& inputHandler,
		                      SDL_Renderer* renderer ) const;

		// TODO Temp method. It shouldn't go here
		void ConfigureCameraComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab ) const;
		void ConfigureHealthComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab ) const;
};
