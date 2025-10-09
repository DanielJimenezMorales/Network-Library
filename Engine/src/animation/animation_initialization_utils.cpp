#include "animation_initialization_utils.h"

#include "asset_manager/asset_manager.h"

#include "animation/animation_asset_loader.h"

#include "ecs/world.h"
#include "ecs/system_coordinator.h"

#include "systems/animation_system.h"

namespace Engine
{
	static bool AddAnimationAssetmanagement( ECS::World& world )
	{
		AssetManager* assetManager = world.GetAssetManager();
		assetManager->RegisterAsset( AssetType::ANIMATION, new AnimationAssetLoader() );
		return true;
	}

	static bool AddAnimationSystems( ECS::World& world )
	{
		Engine::ECS::SystemCoordinator* animation_system_coordinator =
		    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
		Engine::AnimationSystem* animationSystem = new Engine::AnimationSystem( world.GetAssetManager() );
		animation_system_coordinator->AddSystemToTail( animationSystem );
		auto on_configure_animation_callback =
		    std::bind( &Engine::AnimationSystem::ConfigureAnimationComponent, animationSystem, std::placeholders::_1,
		               std::placeholders::_2 );
		world.SubscribeToOnEntityConfigure( on_configure_animation_callback );
		world.AddSystem( animation_system_coordinator );
		return true;
	}

	bool AddAnimationToWorld( Engine::ECS::World& world )
	{
		bool result = AddAnimationAssetmanagement( world );
		if ( !result )
		{
			return false;
		}

		result = AddAnimationSystems( world );
		if ( !result )
		{
			return false;
		}

		return true;
	}
} // namespace Engine
