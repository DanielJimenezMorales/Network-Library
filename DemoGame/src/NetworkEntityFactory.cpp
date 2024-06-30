#include "NetworkEntityFactory.h"
#include "GameEntity.hpp"
#include "SpriteRendererComponent.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"
#include "PlayerMovement.h"
#include "InputComponent.h"
#include "PlayerNetworkComponent.h"
#include "Scene.h"
#include "TextureLoader.h"
#include "PlayerControllerComponent.h"
#include "RemotePlayerControllerComponent.h"

void NetworkEntityFactory::SetTextureLoader(TextureLoader* textureLoader)
{
	_textureLoader = textureLoader;
}

void NetworkEntityFactory::SetScene(Scene* scene)
{
	_scene = scene;
}

void NetworkEntityFactory::SetKeyboard(IInputController* inputController)
{
	_inputController = inputController;
}

void NetworkEntityFactory::SetPeerType(NetLib::PeerType peerType)
{
	_peerType = peerType;
}

int NetworkEntityFactory::CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, float posX, float posY, NetLib::NetworkVariableChangesHandler* networkVariableChangeHandler)
{
	Texture* texture = _textureLoader->LoadTexture("sprites/PlayerSprites/playerHead.png");

	GameEntity entity = _scene->CreateGameEntity();
	TransformComponent& transform = entity.GetComponent<TransformComponent>();
	transform.position = Vec2f(posX, posY);

	entity.AddComponent<SpriteRendererComponent>(texture);

	if (_peerType == NetLib::ServerMode)
	{
		//TODO Make the input component within a dedicated input entity and not specific from one entity. Also, deactivate it on server. Client should send inputs.
		entity.AddComponent<InputComponent>(_inputController);
		PlayerControllerConfiguration playerConfiguration;
		playerConfiguration.movementSpeed = 250;
		entity.AddComponent<PlayerControllerComponent>(networkVariableChangeHandler, networkEntityId, playerConfiguration);
		//entity.AddComponent<ScriptComponent>().Bind<PlayerMovement>();
	}
	else
	{
		entity.AddComponent<RemotePlayerControllerComponent>(networkVariableChangeHandler, networkEntityId);
		//entity.AddComponent<ScriptComponent>().Bind<PlayerDummyMovement>();
	}

	entity.AddComponent<PlayerNetworkComponent>(networkVariableChangeHandler, networkEntityId);

	return entity.GetId();
}

void NetworkEntityFactory::DestroyNetworkEntityObject(uint32_t gameEntity)
{
}
