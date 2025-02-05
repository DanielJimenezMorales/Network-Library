#pragma once
#include "core/peer.h"

#include <SDL_image.h>

class Scene;
class InputHandler;

// TODO TEMP
class GameEntity;
namespace ECS
{
	class Prefab;
}

class SceneInitializer
{
	public:
		void InitializeScene( Scene& scene, NetLib::PeerType networkPeerType, InputHandler& inputHandler,
		                      SDL_Renderer* renderer ) const;

		// TODO Temp method. It shouldn't go here
		void ConfigureCameraComponent( GameEntity& entity, const ECS::Prefab& prefab ) const;
};
