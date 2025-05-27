#include "server_world_initializer.h"

//#define SERVER_RENDER

// Engine
#include "components/transform_component.h"
#include "components/sprite_renderer_component.h"
#include "components/camera_component.h"
#include "components/collider_2d_component.h"
#include "components/gizmo_renderer_component.h"
#include "components/raycast_component.h"

#include "global_components/input_handler_global_component.h"

#include "ecs/system_coordinator.h"
#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "systems/collision_detection_system.h"

#ifdef SERVER_RENDER
	#include "render/rendering_inicialization_utils.h"
#endif

// Network library
#include "core/server.h"
#include "core/initializer.h"

// Shared game
#include "shared/components/network_entity_component.h"
#include "shared/components/player_controller_component.h"
#include "shared/components/temporary_lifetime_component.h"
#include "shared/components/health_component.h"

#include "shared/systems/pre_tick_network_system.h"
#include "shared/systems/pos_tick_network_system.h"
#include "shared/systems/temporary_lifetime_objects_system.h"

// Server game
#include "server/components/server_player_state_storage_component.h"

#include "server/systems/server_player_controller_system.h"
#include "server/systems/server_dummy_input_handler_system.h"
//---

#include "shared/InputActionIdsConfiguration.h"
#include "shared/ITextureLoader.h"
#include "shared/InputStateFactory.h"

#include "shared/component_configurations/player_controller_component_configuration.h"
#include "shared/component_configurations/health_component_configuration.h"
#include "component_configurations/sprite_renderer_component_configuration.h"
#include "component_configurations/collider_2d_component_configuration.h"
#include "component_configurations/camera_component_configuration.h"

#include "shared/global_components/network_peer_global_component.h"

#include "shared/network_entity_creator.h"
#include "shared/json_configuration_loader.h"

#include <SDL.h>

// TODO Make the rule of following this initialization order to avoid issues:
//  1. Register components
//  2. Register archetypes
//  3. Register prefabs
//  4. Register systems
//  5. Populate global entities
//  6. Populate in-game entities

ServerWorldInitializer::ServerWorldInitializer()
    : Engine::IWorldInitializer()
{
}

static void RegisterComponents( Engine::ECS::World& world )
{
	// Engine
	world.RegisterComponent< Engine::TransformComponent >( "Transform" );
	world.RegisterComponent< Engine::Collider2DComponent >( "Collider2D" );
	world.RegisterComponent< Engine::CameraComponent >( "Camera" );
	world.RegisterComponent< Engine::RaycastComponent >( "Raycast" );
	world.RegisterComponent< Engine::SpriteRendererComponent >( "SpriteRenderer" );

	// Shared game
	world.RegisterComponent< NetworkEntityComponent >( "NetworkEntity" );
	world.RegisterComponent< PlayerControllerComponent >( "PlayerController" );
	world.RegisterComponent< TemporaryLifetimeComponent >( "TemporaryLifetime" );
	world.RegisterComponent< HealthComponent >( "HealthComponent" );

	// This is server side only
	world.RegisterComponent< ServerPlayerStateStorageComponent >( "ServerPlayerStateStorage" );
}

static void RegisterArchetypes( Engine::ECS::World& world )
{
	JsonConfigurationLoader configuration_loader;
	std::vector< Engine::ECS::Archetype > loaded_archetypes;
	configuration_loader.LoadArchetypes( loaded_archetypes, "config_files/server/entity_archetypes/" );

	for ( auto cit = loaded_archetypes.cbegin(); cit != loaded_archetypes.cend(); ++cit )
	{
		world.RegisterArchetype( *cit );
	}
}

static void RegisterPrefabs( Engine::ECS::World& world )
{
	JsonConfigurationLoader configuration_loader;
	std::vector< Engine::ECS::Prefab > loaded_prefabs;
	configuration_loader.LoadPrefabs( loaded_prefabs );

	for ( auto it = loaded_prefabs.begin(); it != loaded_prefabs.end(); ++it )
	{
		world.RegisterPrefab( std::move( *it ) );
	}
}

#ifdef SERVER_RENDER
static bool AddRenderingToWorld( Engine::ECS::World& world )
{
	// In order to enable Server-side rendering with SDL we need to also enable SDL inputs and add a dummy input
	// gathering system so Windows doesn't think the program has freezed. I know, it's a bit werid...
	if ( SDL_WasInit( SDL_INIT_EVENTS ) )
	{
		SDL_InitSubSystem( SDL_INIT_EVENTS );
	}

	Engine::ECS::SystemCoordinator* dummyInputsHandlerSystemCoordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::INPUT_HANDLING );
	dummyInputsHandlerSystemCoordinator->AddSystemToTail( new ServerDummyInputHandlerSystem() );
	world.AddSystem( dummyInputsHandlerSystemCoordinator );

	const bool result = Engine::AddRenderingToWorld( world );

	return result;
}
#endif

static bool AddNetworkToWorld( Engine::ECS::World& world )
{
	// Add network peer global component
	NetworkPeerGlobalComponent& networkPeerComponent = world.AddGlobalComponent< NetworkPeerGlobalComponent >();
	NetLib::Server* serverPeer = new NetLib::Server( 2 );
	// TODO Make this initializer internal when calling to start
	NetLib::Initializer::Initialize();
	networkPeerComponent.peer = serverPeer;

	// Create network entity creator system
	NetworkEntityCreatorSystem* network_entity_creator = new NetworkEntityCreatorSystem();
	network_entity_creator->SetScene( &world );
	network_entity_creator->SetPeerType( serverPeer->GetPeerType() );
	world.SubscribeToOnEntityConfigure( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityComponentConfigure,
	                                               network_entity_creator, std::placeholders::_1,
	                                               std::placeholders::_2 ) );
	serverPeer->SubscribeToOnNetworkEntityCreate( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityCreate,
	                                                         network_entity_creator, std::placeholders::_1 ) );
	serverPeer->SubscribeToOnNetworkEntityDestroy( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityDestroy,
	                                                          network_entity_creator, std::placeholders::_1 ) );

	// Register input state factory
	InputStateFactory* inputStateFactory = new InputStateFactory();
	serverPeer->RegisterInputStateFactory( inputStateFactory );
	networkPeerComponent.inputStateFactory = inputStateFactory;
	networkPeerComponent.TrackOnRemotePeerConnect();

	// Pre tick network system
	Engine::ECS::SystemCoordinator* preTickNetworkSystemCoordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::PRETICK );
	preTickNetworkSystemCoordinator->AddSystemToTail( new PreTickNetworkSystem() );
	world.AddSystem( preTickNetworkSystemCoordinator );

	// Post tick network system
	Engine::ECS::SystemCoordinator* posTickNetworkSystemCoordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::POSTICK );
	posTickNetworkSystemCoordinator->AddSystemToTail( new PosTickNetworkSystem() );
	world.AddSystem( posTickNetworkSystemCoordinator );

	return true;
}

static bool AddCollisionsToWorld( Engine::ECS::World& world )
{
	// Add Server-side collision detection system
	Engine::ECS::SystemCoordinator* collision_detection_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::PRETICK );
	Engine::CollisionDetectionSystem* collision_detection_system = new Engine::CollisionDetectionSystem();
	auto on_configure_collider_2d_callback =
	    std::bind( &Engine::CollisionDetectionSystem::ConfigureCollider2DComponent, collision_detection_system,
	               std::placeholders::_1, std::placeholders::_2 );
	world.SubscribeToOnEntityConfigure( on_configure_collider_2d_callback );
	collision_detection_system_coordinator->AddSystemToTail( collision_detection_system );
	world.AddSystem( collision_detection_system_coordinator );

	return true;
}

static bool AddGameplayToWorld( Engine::ECS::World& world )
{
	// Add temporary lifetime objects system
	Engine::ECS::SystemCoordinator* temporary_lifetime_objects_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
	TemporaryLifetimeObjectsSystem* temporary_lifetime_objects_system = new TemporaryLifetimeObjectsSystem();
	auto on_configure_temporary_lifetime_callback =
	    std::bind( &TemporaryLifetimeObjectsSystem::ConfigureTemporaryLifetimeComponent,
	               temporary_lifetime_objects_system, std::placeholders::_1, std::placeholders::_2 );
	world.SubscribeToOnEntityConfigure( on_configure_temporary_lifetime_callback );
	temporary_lifetime_objects_system_coordinator->AddSystemToTail( temporary_lifetime_objects_system );
	world.AddSystem( temporary_lifetime_objects_system_coordinator );

	// Add Server-side player controller system
	Engine::ECS::SystemCoordinator* server_player_controller_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::TICK );
	ServerPlayerControllerSystem* server_player_controller_system = new ServerPlayerControllerSystem();
	server_player_controller_system_coordinator->AddSystemToTail( server_player_controller_system );
	auto on_configure_player_controller_callback =
	    std::bind( &ServerPlayerControllerSystem::ConfigurePlayerControllerComponent, server_player_controller_system,
	               std::placeholders::_1, std::placeholders::_2 );
	world.SubscribeToOnEntityConfigure( on_configure_player_controller_callback );
	world.AddSystem( server_player_controller_system_coordinator );

	return true;
}

static bool CreateSystemsAndGlobalEntities( Engine::ECS::World& world )
{
	// Populate systems
	// TODO Create a system storage in order to be able to free them at the end

	bool result = false;
#ifdef SERVER_RENDER;
	//////////////
	// RENDERING
	//////////////
	result = AddRenderingToWorld( world );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize rendering" );
	}
#endif

	////////////
	// NETWORK
	////////////
	result = AddNetworkToWorld( world );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize network" );
	}

	////////////////
	// COLLISIONS
	///////////////
	result = AddCollisionsToWorld( world );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize collisions" );
	}

	/////////////////////////
	// SERVER-SIDE GAMEPLAY
	/////////////////////////
	result = AddGameplayToWorld( world );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize server-side gameplay" );
	}
	return true;
}

static bool CreateGameEntities( Engine::ECS::World& world )
{
	world.CreateGameEntity( "Camera", Vec2f( 0, 0 ) );

	// Add dummy collider entity
	world.CreateGameEntity( "Dummy", Vec2f( 10.f, 10.f ) );

	return true;
}

void ServerWorldInitializer::SetUpWorld( Engine::ECS::World& world )
{
	RegisterComponents( world );
	RegisterArchetypes( world );
	RegisterPrefabs( world );

	// These subscriptions are also temp until I find a better place for them
	world.SubscribeToOnEntityConfigure( std::bind( &ServerWorldInitializer::ConfigureCameraComponent, this,
	                                               std::placeholders::_1, std::placeholders::_2 ) );
	world.SubscribeToOnEntityConfigure( std::bind( &ServerWorldInitializer::ConfigureHealthComponent, this,
	                                               std::placeholders::_1, std::placeholders::_2 ) );

	CreateSystemsAndGlobalEntities( world );
	CreateGameEntities( world );
}

void ServerWorldInitializer::ConfigureCameraComponent( Engine::ECS::GameEntity& entity,
                                                       const Engine::ECS::Prefab& prefab ) const
{
	auto component_config_found = prefab.componentConfigurations.find( "Camera" );
	if ( component_config_found == prefab.componentConfigurations.end() )
	{
		return;
	}

	if ( !entity.HasComponent< Engine::CameraComponent >() )
	{
		return;
	}

	const Engine::CameraComponentConfiguration& camera_config =
	    static_cast< const Engine::CameraComponentConfiguration& >( *component_config_found->second );
	Engine::CameraComponent& camera = entity.GetComponent< Engine::CameraComponent >();
	camera.width = camera_config.width;
	camera.height = camera_config.height;
}

void ServerWorldInitializer::ConfigureHealthComponent( Engine::ECS::GameEntity& entity,
                                                       const Engine::ECS::Prefab& prefab ) const
{
	auto component_config_found = prefab.componentConfigurations.find( "Health" );
	if ( component_config_found == prefab.componentConfigurations.end() )
	{
		return;
	}

	if ( !entity.HasComponent< HealthComponent >() )
	{
		return;
	}

	const HealthComponentConfiguration& health_config =
	    static_cast< const HealthComponentConfiguration& >( *component_config_found->second );
	HealthComponent& health = entity.GetComponent< HealthComponent >();
	health.maxHealth = health_config.maxHealth;
	health.currentHealth = health_config.currentHealth;
}
