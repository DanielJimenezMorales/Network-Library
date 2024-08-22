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
#include "Client.h"
#include "NetworkEntityComponent.h"

void NetworkEntityFactory::SetScene(Scene* scene)
{
	_scene = scene;
}

//TODO Not useful anymore. Delete
void NetworkEntityFactory::SetPeerType(NetLib::PeerType peerType)
{
	_peerType = peerType;
}

int NetworkEntityFactory::CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, uint32_t controlledByPeerId, float posX, float posY, NetLib::NetworkVariableChangesHandler* networkVariableChangeHandler)
{
	LOG_INFO("CONTROLLER BY PEER ID %u", controlledByPeerId);
	ServiceLocator& serviceLocator = ServiceLocator::GetInstance();
	ITextureLoader& textureLoader = serviceLocator.GetTextureLoader();
	Texture* texture = textureLoader.LoadTexture("sprites/PlayerSprites/playerHead.png");

	const GameEntity networkPeerEntity = _scene->GetFirstEntityOfType<NetworkPeerComponent>();
	const NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();

	GameEntity entity = _scene->CreateGameEntity();
	TransformComponent& transform = entity.GetComponent<TransformComponent>();
	transform.SetPosition(Vec2f(posX, posY));

	entity.AddComponent<SpriteRendererComponent>(texture);

	PlayerControllerConfiguration playerConfiguration;
	playerConfiguration.movementSpeed = 25;

	entity.AddComponent<NetworkEntityComponent>(networkEntityId, controlledByPeerId);

	//For player entities, its network variables IDs will go from 1 to 100 both included.
	networkVariableChangeHandler->SetNextNetworkVariableId(1);

	if (networkPeerComponent.peer->GetPeerType() == NetLib::ServerMode)
	{
		entity.AddComponent<PlayerControllerComponent>(networkVariableChangeHandler, networkEntityId, playerConfiguration);
	}
	else
	{
		const NetLib::Client* clientPeer = static_cast<NetLib::Client*>(networkPeerComponent.peer);
		if (clientPeer->GetLocalClientId() == controlledByPeerId)
		{
			entity.AddComponent<PlayerControllerComponent>(networkVariableChangeHandler, networkEntityId, playerConfiguration);
		}
		else
		{
			LOG_WARNING("ME CREO EL LOCAL");
			entity.AddComponent<RemotePlayerControllerComponent>(networkVariableChangeHandler, networkEntityId);
		}
	}

	//entity.AddComponent<PlayerNetworkComponent>(networkVariableChangeHandler, networkEntityId);

	return entity.GetId();
}

void NetworkEntityFactory::DestroyNetworkEntityObject(uint32_t gameEntity)
{
}
