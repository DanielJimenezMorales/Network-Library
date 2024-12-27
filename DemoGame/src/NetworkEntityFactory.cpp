#include "NetworkEntityFactory.h"
#include "GameEntity.hpp"
#include "SpriteRendererComponent.h"
#include "TransformComponent.h"
#include "PlayerNetworkComponent.h"
#include "Scene.h"
#include "ServiceLocator.h"
#include "ITextureLoader.h"
#include "PlayerControllerComponent.h"
#include "RemotePlayerControllerComponent.h"
#include "NetworkPeerComponent.h"
#include "core/client.h"
#include "NetworkEntityComponent.h"
#include "CircleBounds2D.h"
#include "player_network_entity_serialization_callbacks.h"

#include "Vec2f.h"

#include "replication/network_entity_communication_callbacks.h"

void NetworkEntityFactory::SetScene( Scene* scene )
{
	_scene = scene;
}

// TODO Not useful anymore. Delete
void NetworkEntityFactory::SetPeerType( NetLib::PeerType peerType )
{
	_peerType = peerType;
}

int32 NetworkEntityFactory::CreateNetworkEntityObject(
    uint32 networkEntityType, uint32 networkEntityId, uint32 controlledByPeerId, float32 posX, float32 posY,
    NetLib::NetworkEntityCommunicationCallbacks& communication_callbacks )
{
	LOG_INFO( "CONTROLLER BY PEER ID %u", controlledByPeerId );
	ServiceLocator& serviceLocator = ServiceLocator::GetInstance();
	ITextureLoader& textureLoader = serviceLocator.GetTextureLoader();
	Texture* texture = textureLoader.LoadTexture( "sprites/PlayerSprites/playerHead.png" );

	const GameEntity networkPeerEntity = _scene->GetFirstEntityOfType< NetworkPeerComponent >();
	const NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent< NetworkPeerComponent >();

	GameEntity entity = _scene->CreateGameEntity();
	TransformComponent& transform = entity.GetComponent< TransformComponent >();
	transform.SetPosition( Vec2f( posX, posY ) );

	entity.AddComponent< SpriteRendererComponent >( texture );

	CircleBounds2D* circleBounds2D = new CircleBounds2D( 5.f );
	entity.AddComponent< Collider2DComponent >( circleBounds2D, false, CollisionResponseType::Dynamic );

	PlayerControllerConfiguration playerConfiguration;
	playerConfiguration.movementSpeed = 25;

	entity.AddComponent< NetworkEntityComponent >( networkEntityId, controlledByPeerId );

	if ( networkPeerComponent.peer->GetPeerType() == NetLib::PeerType::SERVER )
	{
		entity.AddComponent< PlayerControllerComponent >( nullptr, networkEntityId, playerConfiguration );

		// Subscribe to Serialize for owner
		auto serialize_owner_callback = [ entity ]( NetLib::Buffer& buffer )
		{
			SerializeForOwner( entity, buffer );
		};
		communication_callbacks.OnSerializeEntityStateForOwner.AddSubscriber( serialize_owner_callback );

		// Subscribe to Serialize for non owner
		auto serialize_non_owner_callback = [ entity ]( NetLib::Buffer& buffer )
		{
			SerializeForNonOwner( entity, buffer );
		};
		communication_callbacks.OnSerializeEntityStateForNonOwner.AddSubscriber( serialize_non_owner_callback );
	}
	else
	{
		const NetLib::Client* clientPeer = static_cast< NetLib::Client* >( networkPeerComponent.peer );
		if ( clientPeer->GetLocalClientId() == controlledByPeerId )
		{
			entity.AddComponent< PlayerControllerComponent >( nullptr, networkEntityId, playerConfiguration );
		}
		else
		{
			LOG_WARNING( "ME CREO EL LOCAL" );
			entity.AddComponent< RemotePlayerControllerComponent >( nullptr, networkEntityId );
		}

		// Subscribe to Serialize for owner
		auto callback = [ entity ]( NetLib::Buffer& buffer ) mutable
		{
			DeserializeForOwner( entity, buffer );
		};
		communication_callbacks.OnUnserializeEntityStateForOwner.AddSubscriber( callback );
	}

	// entity.AddComponent<PlayerNetworkComponent>(networkVariableChangeHandler, networkEntityId);

	return entity.GetId();
}

void NetworkEntityFactory::DestroyNetworkEntityObject( uint32 gameEntity )
{
}
