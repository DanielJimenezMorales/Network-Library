#include "animation_initialization_utils.h"

#include "asset_manager/asset_manager.h"

#include "animation/animation_asset_loader.h"
#include "animation/animation_system.h"

#include "ecs/world.h"
#include "ecs/system_coordinator.h"

#include "game.h"

namespace Engine
{
	static bool AddAnimationAssetmanagement( Game& game )
	{
		AssetManager& assetManager = game.GetAssetManager();
		assetManager.RegisterAsset( AssetType::ANIMATION, new AnimationAssetLoader() );
		return true;
	}

	static bool AddAnimationSystems( Engine::Game& game )
	{
		ECS::World& world = game.GetActiveWorld();
		AssetManager& assetManager = game.GetAssetManager();
		ECS::SystemCoordinator* animation_system_coordinator =
		    new ECS::SystemCoordinator( ECS::ExecutionStage::UPDATE );
		AnimationSystem* animationSystem = new AnimationSystem( &assetManager );
		animation_system_coordinator->AddSystemToTail( animationSystem );
		auto on_configure_animation_callback =
		    std::bind( &AnimationSystem::ConfigureAnimationComponent, animationSystem, std::placeholders::_1,
		               std::placeholders::_2 );
		world.SubscribeToOnEntityConfigure( on_configure_animation_callback );
		world.AddSystem( animation_system_coordinator );
		return true;
	}

	bool AddAnimationToWorld( Engine::Game& game )
	{
		bool result = AddAnimationAssetmanagement( game );
		if ( !result )
		{
			return false;
		}

		result = AddAnimationSystems( game );
		if ( !result )
		{
			return false;
		}

		return true;
	}
} // namespace Engine
