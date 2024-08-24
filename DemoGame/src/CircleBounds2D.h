#pragma once
#include "Collider2DComponent.h"

struct CircleBounds2D : public Bounds2D
{
public:
	CircleBounds2D(float radius) : Bounds2D(CollisionShapeType::Circle), radius(radius)
	{
	}

	void GetAxes(const TransformComponent& transform, std::vector<Vec2f>& outAxes) const override;
	void ProjectAxis(const TransformComponent& transform, const Vec2f& axis, float& outMin, float& outMax) const override;
	Vec2f GetClosestVertex(const TransformComponent& transform, const Vec2f& inputPoint) const override;

	Gizmo* GetGizmo(const TransformComponent& transform) const override;

private:
	float radius;
};
