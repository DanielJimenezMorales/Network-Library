#pragma once

#include <SDL_image.h>

#include "entt.hpp"

struct TransformComponent
{
public:
	TransformComponent() {};
	TransformComponent(float x, float y) : posX(x), posY(y) {};

	float posX;
	float posY;
};

struct SpriteRendererComponent
{
public:
	SpriteRendererComponent() {};
	SpriteRendererComponent(const SDL_Rect& srcRect, SDL_Texture* tex) : sourceRect(srcRect), texture(tex){};

	SDL_Rect sourceRect;
	SDL_Texture* texture;
};

class SpriteRendererSystem
{
public:
	void Render(const entt::registry& registry, SDL_Renderer* renderer)
	{
		SDL_Rect destRect;
		auto& view = registry.view<SpriteRendererComponent, TransformComponent>();

		for (auto entity : view)
		{
			auto [spriteRenderer, transform] = view.get<SpriteRendererComponent, TransformComponent>(entity);
			destRect.x = transform.posX - (spriteRenderer.sourceRect.w / 2);
			destRect.y = transform.posY - (spriteRenderer.sourceRect.h / 2);
			destRect.w = spriteRenderer.sourceRect.w;
			destRect.h = spriteRenderer.sourceRect.h;
			SDL_RenderCopy(renderer, spriteRenderer.texture, &spriteRenderer.sourceRect, &destRect);
		}
	}
};

class Scene
{
public:
	Scene() {};
	~Scene() {};

	void Update(float elapsedTime);
	void Render(SDL_Renderer* renderer);

	entt::registry _registry;

private:
	SpriteRendererSystem _spriteRendererSystem;
};

