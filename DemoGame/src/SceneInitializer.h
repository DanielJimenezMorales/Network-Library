#pragma once
#include "core/peer.h"

#include <SDL_image.h>

class InputHandler;

// TODO TEMP
namespace ECS
{
	class GameEntity;
	class Scene;
	class Prefab;
}

class SceneInitializer
{
	public:
		void InitializeScene( ECS::Scene& scene, NetLib::PeerType networkPeerType, InputHandler& inputHandler,
		                      SDL_Renderer* renderer ) const;

		// TODO Temp method. It shouldn't go here
		void ConfigureCameraComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab ) const;
};
