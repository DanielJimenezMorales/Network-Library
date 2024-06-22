#include "SpriteRendererSystem.h"
#include "TransformComponent.h"
#include "SpriteRendererComponent.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include <vector>

void SpriteRendererSystem::Render(EntityContainer& entityContainer, SDL_Renderer* renderer) const
{
	SDL_Rect destRect;
	std::vector<GameEntity> entitiesToRender = entityContainer.GetEntitiesOfBothTypes<SpriteRendererComponent, TransformComponent>();
	auto cit = entitiesToRender.cbegin();
	for (; cit != entitiesToRender.cend(); ++cit)
	{
		//auto [spriteRenderer, transform] = view.get<SpriteRendererComponent, TransformComponent>(entity);
		const SpriteRendererComponent& spriteRenderer = cit->GetComponent<SpriteRendererComponent>();
		const TransformComponent& transform = cit->GetComponent<TransformComponent>();

		destRect.x = transform.posX - (spriteRenderer.sourceRect.w / 2);
		destRect.y = transform.posY - (spriteRenderer.sourceRect.h / 2);
		destRect.w = spriteRenderer.sourceRect.w;
		destRect.h = spriteRenderer.sourceRect.h;
		SDL_RenderCopy(renderer, spriteRenderer.texture, &spriteRenderer.sourceRect, &destRect);
	}
}
