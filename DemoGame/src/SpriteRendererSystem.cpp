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
		Vec2f screenPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(transform.GetPosition());
		destRect.x = static_cast<int>(screenPosition.X() - (texture->GetDimensions().w / 2.f));
		destRect.y = static_cast<int>(screenPosition.Y() - (texture->GetDimensions().h / 2.f));
		destRect.w = static_cast<int>(texture->GetDimensions().w * transform.GetScale().X());
		destRect.h = static_cast<int>(texture->GetDimensions().h * transform.GetScale().Y());

		//SDL_RenderCopy(renderer, texture->GetRaw(), &texture->GetDimensions(), &destRect);
		SDL_RenderCopyEx(renderer, texture->GetRaw(), &texture->GetDimensions(), &destRect, transform.GetRotationAngle(), nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
	}
}
