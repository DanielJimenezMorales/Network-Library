#include "SceneInitializer.h"

#include "core/client.h"
#include "core/server.h"
#include "core/initializer.h"

#include "inputs/keyboard_controller.h"
#include "inputs/mouse_controller.h"

#include "InputActionIdsConfiguration.h"
#include "ITextureLoader.h"
#include "InputStateFactory.h"

#include "ecs/system_coordinator.h"
#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/transform_component.h"
#include "components/virtual_mouse_component.h"
#include "components/sprite_renderer_component.h"
#include "components/crosshair_component.h"
#include "components/camera_component.h"
#include "components/collider_2d_component.h"
#include "components/gizmo_renderer_component.h"
#include "components/network_entity_component.h"
#include "components/player_controller_component.h"
#include "components/remote_player_controller_component.h"
#include "components/raycast_component.h"
#include "components/temporary_lifetime_component.h"
#include "components/health_component.h"
#include "components/client_side_prediction_component.h"
#include "components/server_player_state_storage_component.h"
#include "components/ghost_object_component.h"
#include "components/interpolated_object_component.h"

#include "component_configurations/sprite_renderer_component_configuration.h"
#include "component_configurations/player_controller_component_configuration.h"
#include "component_configurations/collider_2d_component_configuration.h"
#include "component_configurations/camera_component_configuration.h"
#include "component_configurations/health_component_configuration.h"

#include "global_components/network_peer_global_component.h"
#include "global_components/input_handler_global_component.h"

#include "systems/collision_detection_system.h"

#include "ecs_systems/server_player_controller_system.h"
#include "ecs_systems/client_local_player_predictor_system.h"
#include "ecs_systems/client_local_player_server_reconciliator_system.h"
#include "ecs_systems/remote_player_controller_system.h"
#include "ecs_systems/crosshair_follow_mouse_system.h"
#include "ecs_systems/virtual_mouse_system.h"
#include "ecs_systems/pre_tick_network_system.h"
#include "ecs_systems/pos_tick_network_system.h"
#include "ecs_systems/temporary_lifetime_objects_system.h"
#include "ecs_systems/interpolated_player_objects updater_system.h"

#include "render/rendering_inicialization_utils.h"

#include "inputs/inputs_initialization_utils.h"

#include "network_entity_creator.h"
#include "json_configuration_loader.h"

static void RegisterComponents( Engine::ECS::World& scene )
{
	scene.RegisterComponent< Engine::TransformComponent >( "Transform" );
	scene.RegisterComponent< Engine::SpriteRendererComponent >( "SpriteRenderer" );
	scene.RegisterComponent< Engine::Collider2DComponent >( "Collider2D" );
	scene.RegisterComponent< Engine::CameraComponent >( "Camera" );
	scene.RegisterComponent< VirtualMouseComponent >( "VirtualMouse" );
	scene.RegisterComponent< CrosshairComponent >( "Crosshair" );
	scene.RegisterComponent< NetworkEntityComponent >( "NetworkEntity" );
	scene.RegisterComponent< PlayerControllerComponent >( "PlayerController" );
	scene.RegisterComponent< RemotePlayerControllerComponent >( "RemotePlayerController" );
	scene.RegisterComponent< Engine::RaycastComponent >( "Raycast" );
	scene.RegisterComponent< TemporaryLifetimeComponent >( "TemporaryLifetime" );
	scene.RegisterComponent< HealthComponent >( "HealthComponent" );
	// This is client side only
	scene.RegisterComponent< GhostObjectComponent >( "GhostObject" );
	scene.RegisterComponent< InterpolatedObjectComponent >( "InterpolatedObject" );
	scene.RegisterComponent< ClientSidePredictionComponent >( "ClientSidePrediction" );
	// This is server side only
	scene.RegisterComponent< ServerPlayerStateStorageComponent >( "ServerPlayerStateStorage" );
}

static void RegisterArchetypes( Engine::ECS::World& scene )
{
	JsonConfigurationLoader configuration_loader;
	std::vector< Engine::ECS::Archetype > loaded_archetypes;
	configuration_loader.LoadArchetypes( loaded_archetypes );

	for ( auto cit = loaded_archetypes.cbegin(); cit != loaded_archetypes.cend(); ++cit )
	{
		scene.RegisterArchetype( *cit );
	}
}

static void RegisterPrefabs( Engine::ECS::World& scene )
{
	JsonConfigurationLoader configuration_loader;
	std::vector< Engine::ECS::Prefab > loaded_prefabs;
	configuration_loader.LoadPrefabs( loaded_prefabs );

	for ( auto it = loaded_prefabs.begin(); it != loaded_prefabs.end(); ++it )
	{
		scene.RegisterPrefab( std::move( *it ) );
	}
}

static void RegisterSystems( Engine::ECS::World& scene, NetLib::PeerType networkPeerType )
{
	// Populate systems
	// TODO Create a system storage in order to be able to free them at the end

	///////////////////////////
	// INPUT HANDLING SYSTEMS
	///////////////////////////
	Engine::AddInputsToWorld( scene );

	/////////////////////
	// PRE TICK SYSTEMS
	/////////////////////

	// Add temporary lifetime objects system
	Engine::ECS::SystemCoordinator* temporary_lifetime_objects_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
	TemporaryLifetimeObjectsSystem* temporary_lifetime_objects_system = new TemporaryLifetimeObjectsSystem();
	auto on_configure_temporary_lifetime_callback =
	    std::bind( &TemporaryLifetimeObjectsSystem::ConfigureTemporaryLifetimeComponent,
	               temporary_lifetime_objects_system, std::placeholders::_1, std::placeholders::_2 );
	scene.SubscribeToOnEntityConfigure( on_configure_temporary_lifetime_callback );
	temporary_lifetime_objects_system_coordinator->AddSystemToTail( temporary_lifetime_objects_system );
	scene.AddSystem( temporary_lifetime_objects_system_coordinator );

	if ( networkPeerType == NetLib::PeerType::SERVER )
	{
		/////////////////////
		// PRE TICK SYSTEMS
		/////////////////////

		// Add Server-side collision detection system
		Engine::ECS::SystemCoordinator* collision_detection_system_coordinator =
		    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::PRETICK );
		Engine::CollisionDetectionSystem* collision_detection_system = new Engine::CollisionDetectionSystem();
		auto on_configure_collider_2d_callback =
		    std::bind( &Engine::CollisionDetectionSystem::ConfigureCollider2DComponent, collision_detection_system,
		               std::placeholders::_1, std::placeholders::_2 );
		scene.SubscribeToOnEntityConfigure( on_configure_collider_2d_callback );
		collision_detection_system_coordinator->AddSystemToTail( collision_detection_system );
		scene.AddSystem( collision_detection_system_coordinator );

		//////////////////
		// TICK SYSTEMS
		//////////////////

		// Add Server-side player controller system
		Engine::ECS::SystemCoordinator* server_player_controller_system_coordinator =
		    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::TICK );
		ServerPlayerControllerSystem* server_player_controller_system = new ServerPlayerControllerSystem();
		server_player_controller_system_coordinator->AddSystemToTail( server_player_controller_system );
		auto on_configure_player_controller_callback =
		    std::bind( &ServerPlayerControllerSystem::ConfigurePlayerControllerComponent,
		               server_player_controller_system, std::placeholders::_1, std::placeholders::_2 );
		scene.SubscribeToOnEntityConfigure( on_configure_player_controller_callback );
		scene.AddSystem( server_player_controller_system_coordinator );
	}
	else if ( networkPeerType == NetLib::PeerType::CLIENT )
	{
		//////////////////
		// UPDATE SYSTEMS
		//////////////////

		// Add interpolated player objects system
		Engine::ECS::SystemCoordinator* interpolated_player_objects_system_coordinator =
		    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
		InterpolatedPlayerObjectUpdaterSystem* interpolated_player_objects_system =
		    new InterpolatedPlayerObjectUpdaterSystem();
		interpolated_player_objects_system_coordinator->AddSystemToTail( interpolated_player_objects_system );
		scene.AddSystem( interpolated_player_objects_system_coordinator );

		//////////////////
		// TICK SYSTEMS
		//////////////////

		// Add Client-side player controller system
		Engine::ECS::SystemCoordinator* client_player_controller_system_coordinator =
		    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::TICK );

		ClientLocalPlayerServerReconciliatorSystem* client_local_player_server_reconciliator_system =
		    new ClientLocalPlayerServerReconciliatorSystem();
		client_player_controller_system_coordinator->AddSystemToTail( client_local_player_server_reconciliator_system );

		ClientLocalPlayerPredictorSystem* client_local_player_predictor_system =
		    new ClientLocalPlayerPredictorSystem( &scene );
		client_player_controller_system_coordinator->AddSystemToTail( client_local_player_predictor_system );
		auto on_configure_player_controller_callback =
		    std::bind( &ClientLocalPlayerPredictorSystem::ConfigurePlayerControllerComponent,
		               client_local_player_predictor_system, std::placeholders::_1, std::placeholders::_2 );
		scene.SubscribeToOnEntityConfigure( on_configure_player_controller_callback );
		auto on_configure_client_side_predictor_callback =
		    std::bind( &ClientLocalPlayerPredictorSystem::ConfigureClientSidePredictorComponent,
		               client_local_player_predictor_system, std::placeholders::_1, std::placeholders::_2 );
		scene.SubscribeToOnEntityConfigure( on_configure_client_side_predictor_callback );

		scene.AddSystem( client_player_controller_system_coordinator );

		// Add Client-side remote player controller system
		Engine::ECS::SystemCoordinator* client_remote_player_controller_system_coordinator =
		    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::TICK );
		client_player_controller_system_coordinator->AddSystemToTail( new RemotePlayerControllerSystem() );
		scene.AddSystem( client_remote_player_controller_system_coordinator );
	}

	/////////////////////
	// PRE TICK SYSTEMS
	/////////////////////

	// Add pre-tick network system
	Engine::ECS::SystemCoordinator* pre_tick_network_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::PRETICK );
	pre_tick_network_system_coordinator->AddSystemToTail( new PreTickNetworkSystem() );
	scene.AddSystem( pre_tick_network_system_coordinator );

	/////////////////////
	// POS TICK SYSTEMS
	/////////////////////

	// Add pos-tick network system
	Engine::ECS::SystemCoordinator* pos_tick_network_system_coordinator =
	    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::POSTICK );
	pos_tick_network_system_coordinator->AddSystemToTail( new PosTickNetworkSystem() );
	scene.AddSystem( pos_tick_network_system_coordinator );

	//////////////////
	// RENDER SYSTEMS
	//////////////////
	bool result = Engine::AddRenderingToWorld( scene );
	if ( !result )
	{
		LOG_ERROR( "Can't initialize rendering" );
	}
}

void SceneInitializer::ConfigureCameraComponent( Engine::ECS::GameEntity& entity,
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

void SceneInitializer::ConfigureHealthComponent( Engine::ECS::GameEntity& entity,
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

void SceneInitializer::InitializeScene( Engine::ECS::World& scene, NetLib::PeerType networkPeerType ) const
{
	RegisterComponents( scene );
	RegisterArchetypes( scene );
	RegisterPrefabs( scene );
	RegisterSystems( scene, networkPeerType );

	// These subscriptions are also temp until I find a better place for them
	scene.SubscribeToOnEntityConfigure(
	    std::bind( &SceneInitializer::ConfigureCameraComponent, this, std::placeholders::_1, std::placeholders::_2 ) );
	scene.SubscribeToOnEntityConfigure(
	    std::bind( &SceneInitializer::ConfigureHealthComponent, this, std::placeholders::_1, std::placeholders::_2 ) );

	// Inputs
	Engine::InputHandlerGlobalComponent& inputHandlerGlobalComponent =
	    scene.GetGlobalComponent< Engine::InputHandlerGlobalComponent >();

	Engine::KeyboardController* keyboard = new Engine::KeyboardController();
	Engine::InputAxis axis( HORIZONTAL_AXIS, SDLK_d, SDLK_a );
	keyboard->AddAxisMap( axis );
	Engine::InputAxis axis2( VERTICAL_AXIS, SDLK_w, SDLK_s );
	keyboard->AddAxisMap( axis2 );
	inputHandlerGlobalComponent.controllers[ KEYBOARD_NAME ] = keyboard;
	// inputHandler.AddController( keyboard );

	Engine::MouseController* mouse = new Engine::MouseController();
	const Engine::InputButton mouse_shoot_button( SHOOT_BUTTON, SDL_BUTTON_LEFT );
	mouse->AddButtonMap( mouse_shoot_button );
	inputHandlerGlobalComponent.cursors[ MOUSE_NAME ] = mouse;
	// inputHandler.AddCursor( mouse );

	// Populate entities
	scene.CreateGameEntity( "Camera", Vec2f( 0, 0 ) );

	NetworkPeerGlobalComponent& networkPeerComponent = scene.AddGlobalComponent< NetworkPeerGlobalComponent >();
	NetLib::Peer* networkPeer;
	if ( networkPeerType == NetLib::PeerType::SERVER )
	{
		networkPeer = new NetLib::Server( 2 );
	}
	else if ( networkPeerType == NetLib::PeerType::CLIENT )
	{
		networkPeer = new NetLib::Client( 5 );
	}

	// TODO Make this initializer internal when calling to start
	NetLib::Initializer::Initialize();
	networkPeerComponent.peer = networkPeer;

	NetworkEntityCreatorSystem* network_entity_creator = new NetworkEntityCreatorSystem();
	network_entity_creator->SetScene( &scene );
	network_entity_creator->SetPeerType( networkPeerType );
	scene.SubscribeToOnEntityConfigure( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityComponentConfigure,
	                                               network_entity_creator, std::placeholders::_1,
	                                               std::placeholders::_2 ) );

	// Add dummy collider entity
	scene.CreateGameEntity( "Dummy", Vec2f( 10.f, 10.f ) );

	if ( networkPeer->GetPeerType() == NetLib::PeerType::SERVER )
	{
		NetLib::Server* server_peer = networkPeerComponent.GetPeerAsServer();
		server_peer->SubscribeToOnNetworkEntityCreate( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityCreate,
		                                                          network_entity_creator, std::placeholders::_1 ) );
		server_peer->SubscribeToOnNetworkEntityDestroy( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityDestroy,
		                                                           network_entity_creator, std::placeholders::_1 ) );

		InputStateFactory* inputStateFactory = new InputStateFactory();
		networkPeerComponent.GetPeerAsServer()->RegisterInputStateFactory( inputStateFactory );
		networkPeerComponent.inputStateFactory = inputStateFactory;
		networkPeerComponent.TrackOnRemotePeerConnect();
	}

	if ( networkPeer->GetPeerType() == NetLib::PeerType::CLIENT )
	{
		NetLib::Client* client_peer = networkPeerComponent.GetPeerAsClient();
		client_peer->SubscribeToOnNetworkEntityCreate( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityCreate,
		                                                          network_entity_creator, std::placeholders::_1 ) );
		client_peer->SubscribeToOnNetworkEntityDestroy( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityDestroy,
		                                                           network_entity_creator, std::placeholders::_1 ) );

		// Add virtual mouse
		scene.CreateGameEntity( "VirtualMouse", Vec2f( 0, 0 ) );

		// Add virtual mouse system
		Engine::ECS::SystemCoordinator* virtual_mouse_system_coordinator =
		    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
		virtual_mouse_system_coordinator->AddSystemToTail( new VirtualMouseSystem() );
		scene.AddSystem( virtual_mouse_system_coordinator );

		// Add crosshair if being a client
		scene.CreateGameEntity( "Crosshair", Vec2f( 0, 0 ) );

		// Add crosshair follow mouse system
		Engine::ECS::SystemCoordinator* crosshair_follow_mouse_system_coordinator =
		    new Engine::ECS::SystemCoordinator( Engine::ECS::ExecutionStage::UPDATE );
		crosshair_follow_mouse_system_coordinator->AddSystemToTail( new CrosshairFollowMouseSystem() );
		scene.AddSystem( crosshair_follow_mouse_system_coordinator );
	}
}
