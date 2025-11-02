#pragma once
#include "ecs/i_simple_system.h"

namespace Engine
{
	class AssetManager;

	namespace ECS
	{
		class Prefab;
	}

	class AnimationSystem : public ECS::ISimpleSystem
	{
		public:
			AnimationSystem( AssetManager* asset_manager );

			void Execute( ECS::World& world, float32 elapsed_time ) override;

			void ConfigureAnimationComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

		private:
			AssetManager* _assetManager;
	};
} // namespace Engine
