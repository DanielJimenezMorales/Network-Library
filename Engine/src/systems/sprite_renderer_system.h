#pragma once
#include "ecs/i_simple_system.h"

#include "SDL_image.h"

namespace Engine
{
	class AssetManager;

	namespace ECS
	{
		class Prefab;
	}

	class SpriteRendererSystem : public ECS::ISimpleSystem
	{
		public:
			SpriteRendererSystem( SDL_Renderer* renderer, AssetManager* asset_manager );

			void Execute( ECS::World& world, float32 elapsed_time ) override;

			void ConfigureSpriteRendererComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

		private:
			AssetManager* _assetManager;
	};
} // namespace Engine
