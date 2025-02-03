#include "SceneInitializer.h"
#include "Scene.h"
#include "GameEntity.hpp"
#include "core/client.h"
#include "core/server.h"
#include "core/initializer.h"
#include "KeyboardController.h"
#include "MouseController.h"
#include "InputActionIdsConfiguration.h"
#include "InputHandler.h"
#include "ITextureLoader.h"
#include "InputStateFactory.h"
#include "CircleBounds2D.h"

#include "ecs/system_coordinator.h"

#include "components/transform_component.h"
#include "components/virtual_mouse_component.h"
#include "components/sprite_renderer_component.h"
#include "components/crosshair_component.h"
#include "components/camera_component.h"
#include "components/input_component.h"
#include "components/collider_2d_component.h"
#include "components/gizmo_renderer_component.h"
#include "components/network_entity_component.h"
#include "components/player_controller_component.h"
#include "components/remote_player_controller_component.h"

#include "component_configurations/sprite_renderer_component_configuration.h"

#include "global_components/network_peer_global_component.h"

#include "ecs_systems/server_player_controller_system.h"
#include "ecs_systems/client_player_controller_system.h"
#include "ecs_systems/remote_player_controller_system.h"
#include "ecs_systems/crosshair_follow_mouse_system.h"
#include "ecs_systems/sprite_renderer_system.h"
#include "ecs_systems/gizmo_renderer_system.h"
#include "ecs_systems/virtual_mouse_system.h"
#include "ecs_systems/pre_tick_network_system.h"
#include "ecs_systems/pos_tick_network_system.h"
#include "ecs_systems/collision_detection_system.h"

#include "network_entity_creator.h"

void SceneInitializer::InitializeScene( Scene& scene, NetLib::PeerType networkPeerType, InputHandler& inputHandler,
                                        SDL_Renderer* renderer ) const
{
	// Register components
	scene.RegisterComponent< TransformComponent >( "Transform" );
	scene.RegisterComponent< SpriteRendererComponent >( "SpriteRenderer" );
	scene.RegisterComponent< Collider2DComponent >( "Collider2D" );
	scene.RegisterComponent< CameraComponent >( "Camera" );
	scene.RegisterComponent< CrosshairComponent >( "Crosshair" );
	scene.RegisterComponent< NetworkEntityComponent >( "NetworkEntity" );
	scene.RegisterComponent< PlayerControllerComponent >( "PlayerController" );
	scene.RegisterComponent< RemotePlayerControllerComponent >( "RemotePlayerController" );

	// Register archetypes
	ECS::Archetype crosshair_archetype;
	crosshair_archetype.name.assign( "Crosshair" );
	crosshair_archetype.components.push_back( "Transform" );
	crosshair_archetype.components.push_back( "SpriteRenderer" );
	crosshair_archetype.components.push_back( "Crosshair" );
	scene.RegisterArchetype( crosshair_archetype );

	ECS::Archetype player_archetype;
	player_archetype.name.assign( "Player" );
	player_archetype.components.push_back( "Transform" );
	player_archetype.components.push_back( "SpriteRenderer" );
	player_archetype.components.push_back( "Collider2D" );
	player_archetype.components.push_back( "NetworkEntity" );
	player_archetype.components.push_back( "PlayerController" );
	scene.RegisterArchetype( player_archetype );

	ECS::Archetype remote_player_archetype;
	remote_player_archetype.name.assign( "RemotePlayer" );
	remote_player_archetype.components.push_back( "Transform" );
	remote_player_archetype.components.push_back( "SpriteRenderer" );
	remote_player_archetype.components.push_back( "Collider2D" );
	remote_player_archetype.components.push_back( "NetworkEntity" );
	remote_player_archetype.components.push_back( "RemotePlayerController" );
	scene.RegisterArchetype( remote_player_archetype );

	// Register prefabs
	ECS::Prefab crosshair_prefab;
	crosshair_prefab.name.assign( "Crosshair" );
	crosshair_prefab.archetype.assign( "Crosshair" );
	SpriteRendererComponentConfiguration* sprite_renderer_config =
	    new SpriteRendererComponentConfiguration( "sprites/Crosshair/crosshair.png" );
	crosshair_prefab.componentConfigurations[ sprite_renderer_config->name ] = sprite_renderer_config;
	scene.RegisterPrefab( std::move( crosshair_prefab ) );

	ECS::Prefab player_prefab;
	player_prefab.name.assign( "Player" );
	player_prefab.archetype.assign( "Player" );
	SpriteRendererComponentConfiguration* player_sprite_renderer_config =
	    new SpriteRendererComponentConfiguration( "sprites/PlayerSprites/playerHead.png" );
	player_prefab.componentConfigurations[ player_sprite_renderer_config->name ] = player_sprite_renderer_config;
	PlayerControllerConfiguration* player_controller_config = new PlayerControllerConfiguration( 25 );
	player_prefab.componentConfigurations[ player_controller_config->name ] = player_controller_config;
	scene.RegisterPrefab( std::move( player_prefab ) );

	ECS::Prefab remote_player_prefab;
	remote_player_prefab.name.assign( "RemotePlayer" );
	remote_player_prefab.archetype.assign( "RemotePlayer" );
	SpriteRendererComponentConfiguration* remote_player_sprite_renderer_config =
	    new SpriteRendererComponentConfiguration( "sprites/PlayerSprites/playerHead.png" );
	remote_player_prefab.componentConfigurations[ remote_player_sprite_renderer_config->name ] =
	    remote_player_sprite_renderer_config;
	scene.RegisterPrefab( std::move( remote_player_prefab ) );

	SpriteRendererSystem* sprite_renderer_system = new SpriteRendererSystem( renderer );
	auto on_configure_sprite_renderer_callback =
	    std::bind( &SpriteRendererSystem::ConfigureSpriteRendererComponent, sprite_renderer_system,
	               std::placeholders::_1, std::placeholders::_2 );
	scene.SubscribeToOnEntityConfigure( on_configure_sprite_renderer_callback );
	GizmoRendererSystem* gizmo_renderer_system = new GizmoRendererSystem( renderer );

	// Inputs
	KeyboardController* keyboard = new KeyboardController();
	InputButton button( JUMP_BUTTON, SDLK_q );
	keyboard->AddButtonMap( button );
	InputAxis axis( HORIZONTAL_AXIS, SDLK_d, SDLK_a );
	keyboard->AddAxisMap( axis );
	InputAxis axis2( VERTICAL_AXIS, SDLK_w, SDLK_s );
	keyboard->AddAxisMap( axis2 );
	inputHandler.AddController( keyboard );

	MouseController* mouse = new MouseController();
	InputButton mouseButton( MOUSE_LEFT_CLICK_BUTTON, SDL_BUTTON_LEFT );
	mouse->AddButtonMap( mouseButton );
	inputHandler.AddCursor( mouse );

	// Populate entities
	GameEntity mainCameraEntity = scene.CreateGameEntity();
	// TODO Do not hardcode width and height values
	mainCameraEntity.AddComponent< CameraComponent >( Vec2f( 0.f, 0.f ), 512, 512 );

	GameEntity inputsEntity = scene.CreateGameEntity();
	inputsEntity.AddComponent< InputComponent >( keyboard, mouse );

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

		// Add dummy collider entity
		GameEntity colliderEntity = scene.CreateGameEntity();
		TransformComponent& colliderEntityTransform = colliderEntity.GetComponent< TransformComponent >();
		colliderEntityTransform.SetPosition( Vec2f( 10.f, 10.f ) );

		TextureHandler texture_handler =
		    sprite_renderer_system->GetTextureResourceHandler()->LoadTexture( "sprites/PlayerSprites/PlayerHead.png" );
		colliderEntity.AddComponent< SpriteRendererComponent >( texture_handler );

		CircleBounds2D* circleBounds2D = new CircleBounds2D( 5.f );
		colliderEntity.AddComponent< Collider2DComponent >( circleBounds2D, false, CollisionResponseType::Static );

		const GizmoHandler& gizmo_handler =
		    gizmo_renderer_system->GetGizmoResourceHandler().CreateGizmo( circleBounds2D->GetGizmo().get() );
		colliderEntity.AddComponent< GizmoRendererComponent >( gizmo_handler );
	}

	if ( networkPeer->GetPeerType() == NetLib::PeerType::CLIENT )
	{
		NetLib::Client* client_peer = networkPeerComponent.GetPeerAsClient();
		client_peer->SubscribeToOnNetworkEntityCreate( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityCreate,
		                                                          network_entity_creator, std::placeholders::_1 ) );
		client_peer->SubscribeToOnNetworkEntityDestroy( std::bind( &NetworkEntityCreatorSystem::OnNetworkEntityDestroy,
		                                                           network_entity_creator, std::placeholders::_1 ) );

		// Add virtual mouse
		GameEntity virtualMouse = scene.CreateGameEntity();
		virtualMouse.AddComponent< VirtualMouseComponent >();

		// Add virtual mouse system
		ECS::SystemCoordinator* virtual_mouse_system_coordinator =
		    new ECS::SystemCoordinator( ECS::ExecutionStage::UPDATE );
		virtual_mouse_system_coordinator->AddSystemToTail( new VirtualMouseSystem() );
		scene.AddSystem( virtual_mouse_system_coordinator );

		// Add crosshair if being a client
		scene.CreateGameEntity( "Crosshair", Vec2f( 0, 0 ) );

		// Add crosshair follow mouse system
		ECS::SystemCoordinator* crosshair_follow_mouse_system_coordinator =
		    new ECS::SystemCoordinator( ECS::ExecutionStage::UPDATE );
		crosshair_follow_mouse_system_coordinator->AddSystemToTail( new CrosshairFollowMouseSystem() );
		scene.AddSystem( crosshair_follow_mouse_system_coordinator );
	}

	// Populate systems
	// TODO Create a system storage in order to be able to free them at the end
	if ( networkPeerType == NetLib::PeerType::SERVER )
	{
		/////////////////////
		// PRE TICK SYSTEMS
		/////////////////////

		// Add Server-side collision detection system
		ECS::SystemCoordinator* collision_detection_system_coordinator =
		    new ECS::SystemCoordinator( ECS::ExecutionStage::PRETICK );
		collision_detection_system_coordinator->AddSystemToTail( new CollisionDetectionSystem() );
		scene.AddSystem( collision_detection_system_coordinator );

		//////////////////
		// TICK SYSTEMS
		//////////////////

		// Add Server-side player controller system
		ECS::SystemCoordinator* server_player_controller_system_coordinator =
		    new ECS::SystemCoordinator( ECS::ExecutionStage::TICK );
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
		// TICK SYSTEMS
		//////////////////

		// Add Client-side player controller system
		ECS::SystemCoordinator* client_player_controller_system_coordinator =
		    new ECS::SystemCoordinator( ECS::ExecutionStage::TICK );
		client_player_controller_system_coordinator->AddSystemToTail( new ClientPlayerControllerSystem() );
		scene.AddSystem( client_player_controller_system_coordinator );

		// Add Client-side remote player controller system
		ECS::SystemCoordinator* client_remote_player_controller_system_coordinator =
		    new ECS::SystemCoordinator( ECS::ExecutionStage::TICK );
		client_player_controller_system_coordinator->AddSystemToTail( new RemotePlayerControllerSystem() );
		scene.AddSystem( client_remote_player_controller_system_coordinator );
	}

	/////////////////////
	// PRE TICK SYSTEMS
	/////////////////////

	// Add pre-tick network system
	ECS::SystemCoordinator* pre_tick_network_system_coordinator =
	    new ECS::SystemCoordinator( ECS::ExecutionStage::PRETICK );
	pre_tick_network_system_coordinator->AddSystemToTail( new PreTickNetworkSystem() );
	scene.AddSystem( pre_tick_network_system_coordinator );

	/////////////////////
	// POS TICK SYSTEMS
	/////////////////////

	// Add pos-tick network system
	ECS::SystemCoordinator* pos_tick_network_system_coordinator =
	    new ECS::SystemCoordinator( ECS::ExecutionStage::POSTICK );
	pos_tick_network_system_coordinator->AddSystemToTail( new PosTickNetworkSystem() );
	scene.AddSystem( pos_tick_network_system_coordinator );

	//////////////////
	// RENDER SYSTEMS
	//////////////////

	ECS::SystemCoordinator* render_system_coordinator = new ECS::SystemCoordinator( ECS::ExecutionStage::RENDER );
	render_system_coordinator->AddSystemToTail( sprite_renderer_system );

	render_system_coordinator->AddSystemToTail( gizmo_renderer_system );
	scene.AddSystem( render_system_coordinator );
	scene.SubscribeToOnEntityCreate( std::bind( &GizmoRendererSystem::AllocateGizmoRendererComponentIfHasCollider,
	                                            gizmo_renderer_system, std::placeholders::_1 ) );
	scene.SubscribeToOnEntityDestroy( std::bind( &GizmoRendererSystem::DeallocateGizmoRendererComponentIfHasCollider,
	                                             gizmo_renderer_system, std::placeholders::_1 ) );
}
