#include "NetworkEntityFactory.h"
#include "GameEntity.hpp"
#include "SDL.h"
#include "SpriteRendererComponent.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"
#include "PlayerMovement.h"
#include "InputComponent.h"
#include "PlayerNetworkComponent.h"

void NetworkEntityFactory::SetRenderer(SDL_Renderer* renderer)
{
	_renderer = renderer;
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
	SDL_Surface* imageSurface = IMG_Load("sprites/PlayerSprites/playerHead.png");

	SDL_Texture* imageTexture = SDL_CreateTextureFromSurface(_renderer, imageSurface);
	SDL_FreeSurface(imageSurface);

	SDL_Rect sourceTextureRect;
	int result = SDL_QueryTexture(imageTexture, NULL, NULL, &sourceTextureRect.w, &sourceTextureRect.h);

	sourceTextureRect.x = 0;
	sourceTextureRect.y = 0;

	GameEntity entity = _scene->CreateGameEntity();
	TransformComponent& transform = entity.GetComponent<TransformComponent>();
	transform.posX = posX;
	transform.posY = posY;

	entity.AddComponent<SpriteRendererComponent>(sourceTextureRect, imageTexture);

	if (_peerType == NetLib::ServerMode)
	{
		entity.AddComponent<InputComponent>(_inputController);
		entity.AddComponent<ScriptComponent>().Bind<PlayerMovement>();
	}
	else
	{
		entity.AddComponent<ScriptComponent>().Bind<PlayerDummyMovement>();
	}

	entity.AddComponent<PlayerNetworkComponent>(networkVariableChangeHandler, networkEntityId);

	return entity.GetId();
}

void NetworkEntityFactory::DestroyNetworkEntityObject(uint32_t gameEntity)
{
}
