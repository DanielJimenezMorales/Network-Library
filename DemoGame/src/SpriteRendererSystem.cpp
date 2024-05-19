#include "SpriteRendererSystem.h"
#include "TransformComponent.h"
#include "SpriteRendererComponent.h"

void SpriteRendererSystem::Render(const entt::registry& registry, SDL_Renderer* renderer) const
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
