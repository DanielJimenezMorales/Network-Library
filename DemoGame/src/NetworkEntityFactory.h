#pragma once
#include "INetworkEntityFactory.h"
#include "Scene.h"
#include "SDL.h"

class NetworkEntityFactory : public NetLib::INetworkEntityFactory
{
public:
	NetworkEntityFactory() {}
	void SetRenderer(SDL_Renderer* renderer);
	void SetScene(Scene* scene);
	int CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, float posX, float posY) override;
	void DestroyNetworkEntityObject(uint32_t gameEntity) override;

private:
	Scene* _scene;
	SDL_Renderer* _renderer;
};
