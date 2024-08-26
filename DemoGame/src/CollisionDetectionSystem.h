#pragma once
#include "IPreTickSystem.h"
#include "Vec2f.h"
#include <vector>

struct Collider2DComponent;
struct TransformComponent;

struct MinimumTranslationVector
{
public:
	MinimumTranslationVector() : direction(), magnitude(0.f)
	{
	}

	MinimumTranslationVector(const Vec2f& direction, float magnitude) : direction(direction), magnitude(magnitude)
	{
	}

	Vec2f direction;
	float magnitude;
};

class CollisionDetectionSystem : public IPreTickSystem
{
public:
	void PreTick(EntityContainer& entityContainer, float elapsedTime) const override;

private:
	bool AreTwoShapesColliding(const Collider2DComponent& collider1, const TransformComponent& transform1, const Collider2DComponent& collider2, const TransformComponent& transform2, MinimumTranslationVector& outMtv) const;
	void GetAllAxes(const Collider2DComponent& collider1, const TransformComponent& transform1, const Collider2DComponent& collider2, const TransformComponent& transform2, std::vector<Vec2f>& outAxesVector) const;
	void NormalizeAxes(std::vector<Vec2f>& axesVector) const;
	bool DoProjectionsOverlap(float minProjection1, float maxProjection1, float minProjection2, float maxProjection2) const;
	float GetProjectionsOverlapMagnitude(float minProjection1, float maxProjection1, float minProjection2, float maxProjection2) const;
};
