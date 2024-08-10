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

#include <cmath>

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

		double rotationAngles = 0;

		float dotProduct = (transform.lookAtDirection.X() * transform.previousLookAtDirection.X()) + (transform.lookAtDirection.Y() * transform.previousLookAtDirection.Y());
		float anglesInRadians = std::acosf(dotProduct);
		rotationAngles = anglesInRadians * (180.0 / M_PI);

		//SDL_RenderCopy(renderer, texture->GetRaw(), &texture->GetDimensions(), &destRect);
		SDL_RenderCopyEx(renderer, texture->GetRaw(), &texture->GetDimensions(), &destRect, rotationAngles, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
	}
}
