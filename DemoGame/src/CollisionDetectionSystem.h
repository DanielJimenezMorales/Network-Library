#pragma once
#include "IPreTickSystem.h"

struct Collider2DComponent;
struct TransformComponent;

class CollisionDetectionSystem : public IPreTickSystem
{
public:
	void PreTick(EntityContainer& entityContainer, float elapsedTime) const override;
};

bool AreTwoShapesColliding(const Collider2DComponent& collider1, const TransformComponent& transform1, const Collider2DComponent& collider2, const TransformComponent& transform2);
