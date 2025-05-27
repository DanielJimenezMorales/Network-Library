#include "client_world_initializer.h"

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

#include "inputs/inputs_initialization_utils.h"
#include "inputs/keyboard_controller.h"
#include "inputs/mouse_controller.h"

#include "render/rendering_inicialization_utils.h"

// Network library
#include "core/client.h"
#include "core/initializer.h"

// Shared game
#include "shared/components/network_entity_component.h"
#include "shared/components/player_controller_component.h"
#include "shared/components/temporary_lifetime_component.h"
#include "shared/components/health_component.h"

#include "shared/systems/pre_tick_network_system.h"
#include "shared/systems/pos_tick_network_system.h"
#include "shared/systems/temporary_lifetime_objects_system.h"

// Client game
#include "client/components/ghost_object_component.h"
#include "client/components/remote_player_controller_component.h"
#include "client/components/client_side_prediction_component.h"
#include "client/components/interpolated_object_component.h"
#include "client/components/virtual_mouse_component.h"
#include "client/components/crosshair_component.h"

#include "client/systems/crosshair_follow_mouse_system.h"
#include "client/systems/client_local_player_server_reconciliator_system.h"
#include "client/systems/virtual_mouse_system.h"
#include "client/systems/client_local_player_predictor_system.h"
#include "client/systems/remote_player_controller_system.h"
#include "client/systems/interpolated_player_objects updater_system.h"
//---

#include "InputActionIdsConfiguration.h"
#include "ITextureLoader.h"
#include "InputStateFactory.h"

#include "component_configurations/sprite_renderer_component_configuration.h"
#include "component_configurations/player_controller_component_configuration.h"
#include "component_configurations/collider_2d_component_configuration.h"
#include "component_configurations/camera_component_configuration.h"
#include "component_configurations/health_component_configuration.h"

#include "global_components/network_peer_global_component.h"

#include "network_entity_creator.h"
#include "json_configuration_loader.h"

ClientWorldInitializer::ClientWorldInitializer()
{
}

static void RegisterComponents( Engine::ECS::World& world )
{
	// Engine
	world.RegisterComponent< Engine::TransformComponent >( "Transform" );
	world.RegisterComponent< Engine::SpriteRendererComponent >( "SpriteRenderer" );
	world.RegisterComponent< Engine::Collider2DComponent >( "Collider2D" );
	world.RegisterComponent< Engine::CameraComponent >( "Camera" );
	world.RegisterComponent< Engine::RaycastComponent >( "Raycast" );

	// Shared game
	world.RegisterComponent< NetworkEntityComponent >( "NetworkEntity" );
	world.RegisterComponent< PlayerControllerComponent >( "PlayerController" );
	world.RegisterComponent< TemporaryLifetimeComponent >( "TemporaryLifetime" );
	world.RegisterComponent< HealthComponent >( "HealthComponent" );

	// Client game
	world.RegisterComponent< RemotePlayerControllerComponent >( "RemotePlayerController" );
	world.RegisterComponent< VirtualMouseComponent >( "VirtualMouse" );
	world.RegisterComponent< CrosshairComponent >( "Crosshair" );
	world.RegisterComponent< GhostObjectComponent >( "GhostObject" );
	world.RegisterComponent< InterpolatedObjectComponent >( "InterpolatedObject" );
	world.RegisterComponent< ClientSidePredictionComponent >( "ClientSidePrediction" );
}

static void RegisterArchetypes( Engine::ECS::World& world )
{
	JsonConfigurationLoader configuration_loader;
	std::vector< Engine::ECS::Archetype > loaded_archetypes;
	configuration_loader.LoadArchetypes( loaded_archetypes, "config_files/client/entity_archetypes/" );

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

static bool AddInputsToWorld( Engine::ECS::World& world )
{
	bool result = Engine::AddInputsToWorld( world );
	if ( !result )
	{
		return false;
	}

	// Inputs
	Engine::InputHandlerGlobalComponent& inputHandlerGlobalComponent =
	    world.GetGlobalComponent< Engine::InputHandlerGlobalComponent >();

	Engine::KeyboardController* keyboard = new Engine::KeyboardController();
	Engine::InputAxis axis( HORIZONTAL_AXIS, SDLK_d, SDLK_a );
	keyboard->AddAxisMap( axis );
	Engine::InputAxis axis2( VERTICAL_AXIS, SDLK_w, SDLK_s );
	keyboard->AddAxisMap( axis2 );
	inputHandlerGlobalComponent.controllers[ std::string( KEYBOARD_NAME ) ] = keyboard;

	Engine::MouseController* mouse = new Engine::MouseController();
	const Engine::InputButton mouse_shoot_button( SHOOT_BUTTON, SDL_BUTTON_LEFT );
	mouse->AddButtonMap( mouse_shoot_button );
	inputHandlerGlobalComponent.cursors[ std::string( MOUSE_NAME ) ] = mouse;

	return true;
}

static bool AddRenderingToWorld( Engine::ECS::World& world )
{
	bool result = Engine::AddRenderingToWorld( world );
	if ( !result )
	{
		return false;
	}

	return true;
}

static bool AddNetworkToWorld( Engine::ECS::World& world )
{
	// Add network peer global component
	NetworkPeerGlobalComponent& networkPeerComponent = world.AddGlobalComponent< NetworkPeerGlobalComponent >();
	NetLib::Client* clientPeer = new NetLib::Client( 5 );
	// TODO Make this initializer internal when calling to start
	NetLib::Initializer::Initialize();
	networkPeerComponent.peer = clientPeer;

	// Create network entity creator system
	NetworkEntityCreatorSystem* network_entity_creator = new NetworkEntityCreatorSystem();
	network_entity_creator->SetScene( &world );
	network_entity_creator->SetPeerType( clientPeer->GetPeerType() );
	world.SubscribeToOnEntityConfigure( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityComponentConfigure,
	                                               network_entity_creator, std::placeholders::_1,
	                                               std::placeholders::_2 ) );
	clientPeer->SubscribeToOnNetworkEntityCreate( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityCreate,
	                                                         network_entity_creator, std::placeholders::_1 ) );
	clientPeer->SubscribeToOnNetworkEntityDestroy( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityDestroy,
	                                                          network_entity_creator, std::placeholders::_1 ) );

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

static bool AddGameplayToWorld( Engine::ECS::World& world )
{
	// Add virtual mouse system
	Engine::ECS::SystemCoordinator* virtual_mouse_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
	virtual_mouse_system_coordinator->AddSystemToTail( new VirtualMouseSystem() );
	world.AddSystem( virtual_mouse_system_coordinator );

	// Add crosshair follow mouse system
	Engine::ECS::SystemCoordinator* crosshair_follow_mouse_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
	crosshair_follow_mouse_system_coordinator->AddSystemToTail( new CrosshairFollowMouseSystem() );
	world.AddSystem( crosshair_follow_mouse_system_coordinator );

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

	// Add interpolated player objects system
	Engine::ECS::SystemCoordinator* interpolated_player_objects_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
	InterpolatedPlayerObjectUpdaterSystem* interpolated_player_objects_system =
	    new InterpolatedPlayerObjectUpdaterSystem();
	interpolated_player_objects_system_coordinator->AddSystemToTail( interpolated_player_objects_system );
	world.AddSystem( interpolated_player_objects_system_coordinator );

	// Add Client-side player controller system
	Engine::ECS::SystemCoordinator* client_player_controller_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::TICK );

	// Reconciliator
	ClientLocalPlayerServerReconciliatorSystem* client_local_player_server_reconciliator_system =
	    new ClientLocalPlayerServerReconciliatorSystem();
	client_player_controller_system_coordinator->AddSystemToTail( client_local_player_server_reconciliator_system );

	// Predictor
	ClientLocalPlayerPredictorSystem* client_local_player_predictor_system =
	    new ClientLocalPlayerPredictorSystem( &world );
	client_player_controller_system_coordinator->AddSystemToTail( client_local_player_predictor_system );
	auto on_configure_player_controller_callback =
	    std::bind( &ClientLocalPlayerPredictorSystem::ConfigurePlayerControllerComponent,
	               client_local_player_predictor_system, std::placeholders::_1, std::placeholders::_2 );
	world.SubscribeToOnEntityConfigure( on_configure_player_controller_callback );
	auto on_configure_client_side_predictor_callback =
	    std::bind( &ClientLocalPlayerPredictorSystem::ConfigureClientSidePredictorComponent,
	               client_local_player_predictor_system, std::placeholders::_1, std::placeholders::_2 );
	world.SubscribeToOnEntityConfigure( on_configure_client_side_predictor_callback );

	world.AddSystem( client_player_controller_system_coordinator );

	// Add Client-side remote player controller system
	Engine::ECS::SystemCoordinator* client_remote_player_controller_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::TICK );
	client_player_controller_system_coordinator->AddSystemToTail( new RemotePlayerControllerSystem() );
	world.AddSystem( client_remote_player_controller_system_coordinator );

	return true;
}

static bool CreateSystemsAndGlobalEntities( Engine::ECS::World& world )
{
	///////////////////
	// INPUT HANDLING
	///////////////////
	bool result = AddInputsToWorld( world );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize input handling" );
	}

	//////////////
	// RENDERING
	//////////////
	result = AddRenderingToWorld( world );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize rendering" );
	}

	////////////
	// NETWORK
	////////////
	result = AddNetworkToWorld( world );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize network" );
	}

	/////////////////////////
	// CLIENT-SIDE GAMEPLAY
	/////////////////////////
	result = AddGameplayToWorld( world );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize client-side gameplay" );
	}

	return true;
}

static bool CreateGameEntities( Engine::ECS::World& world )
{
	world.CreateGameEntity( "Camera", Vec2f( 0, 0 ) );

	// Add crosshair entity
	world.CreateGameEntity( "Crosshair", Vec2f( 0, 0 ) );

	// Add dummy collider entity
	world.CreateGameEntity( "Dummy", Vec2f( 10.f, 10.f ) );

	// Add virtual mouse entity
	world.CreateGameEntity( "VirtualMouse", Vec2f( 0, 0 ) );

	return true;
}

void ClientWorldInitializer::SetUpWorld( Engine::ECS::World& world )
{
	RegisterComponents( world );
	RegisterArchetypes( world );
	RegisterPrefabs( world );

	// Populate systems
	// TODO Create a system storage in order to be able to free them at the end

	// These subscriptions are also temp until I find a better place for them
	world.SubscribeToOnEntityConfigure( std::bind( &ClientWorldInitializer::ConfigureCameraComponent, this,
	                                               std::placeholders::_1, std::placeholders::_2 ) );
	world.SubscribeToOnEntityConfigure( std::bind( &ClientWorldInitializer::ConfigureHealthComponent, this,
	                                               std::placeholders::_1, std::placeholders::_2 ) );

	CreateSystemsAndGlobalEntities( world );
	CreateGameEntities( world );
}

void ClientWorldInitializer::ConfigureCameraComponent( Engine::ECS::GameEntity& entity,
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

void ClientWorldInitializer::ConfigureHealthComponent( Engine::ECS::GameEntity& entity,
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
