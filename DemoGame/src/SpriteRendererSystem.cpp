#include "SpriteRendererSystem.h"
#include "TransformComponent.h"
#include "SpriteRendererComponent.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include <vector>
#include "Texture.h"
#include "CameraComponent.h"
#include "Vec2f.h"
#include "Logger.h"

void SpriteRendererSystem::Render(EntityContainer& entityContainer, SDL_Renderer* renderer) const
{
	const GameEntity cameraEntity = entityContainer.GetFirstEntityOfType<CameraComponent>();
	const CameraComponent& cameraComponent = cameraEntity.GetComponent<CameraComponent>();

	SDL_Rect destRect;
	std::vector<GameEntity> entitiesToRender = entityContainer.GetEntitiesOfBothTypes<SpriteRendererComponent, TransformComponent>();
	auto cit = entitiesToRender.cbegin();
	for (; cit != entitiesToRender.cend(); ++cit)
	{
		//auto [spriteRenderer, transform] = view.get<SpriteRendererComponent, TransformComponent>(entity);
		const SpriteRendererComponent& spriteRenderer = cit->GetComponent<SpriteRendererComponent>();
		const TransformComponent& transform = cit->GetComponent<TransformComponent>();

		Texture* texture = spriteRenderer.texture;
		Vec2f screenPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(transform.position);
		destRect.x = screenPosition.X() - (texture->GetDimensions().w / 2);
		destRect.y = screenPosition.Y() - (texture->GetDimensions().h / 2);
		destRect.w = texture->GetDimensions().w;
		destRect.h = texture->GetDimensions().h;
		SDL_RenderCopy(renderer, texture->GetRaw(), &texture->GetDimensions(), &destRect);
	}
}
