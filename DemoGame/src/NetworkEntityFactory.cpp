#include "NetworkEntityFactory.h"
#include "GameEntity.h"
#include "SDL.h"
#include "SpriteRendererComponent.h"
#include "TransformComponent.h"

void NetworkEntityFactory::SetRenderer(SDL_Renderer* renderer)
{
    _renderer = renderer;
}

void NetworkEntityFactory::SetScene(Scene* scene)
{
    _scene = scene;
}

int NetworkEntityFactory::CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, float posX, float posY)
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
	return entity.GetId();
}

void NetworkEntityFactory::DestroyNetworkEntityObject(uint32_t gameEntity)
{
}
