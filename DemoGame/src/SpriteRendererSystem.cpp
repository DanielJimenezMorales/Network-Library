#include "SpriteRendererSystem.h"
#include "TransformComponent.h"
#include "SpriteRendererComponent.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include <vector>
#include "Texture.h"

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

		Texture* texture = spriteRenderer.texture;
		destRect.x = transform.position.X() - (texture->GetDimensions().w / 2);
		destRect.y = transform.position.Y() - (texture->GetDimensions().h / 2);
		destRect.w = texture->GetDimensions().w;
		destRect.h = texture->GetDimensions().h;
		SDL_RenderCopy(renderer, texture->GetRaw(), &texture->GetDimensions(), &destRect);
	}
}
