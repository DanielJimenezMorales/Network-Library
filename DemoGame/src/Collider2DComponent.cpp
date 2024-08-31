#include "Collider2DComponent.h"

void Collider2DComponent::GetAxes(const TransformComponent& transform, std::vector<Vec2f>& outAxes) const
{
	_bounds->GetAxes(transform, outAxes);
}

void Collider2DComponent::ProjectAxis(const TransformComponent& transform, const Vec2f& axis, float& outMin, float& outMax) const
{
	_bounds->ProjectAxis(transform, axis, outMin, outMax);
}

Vec2f Collider2DComponent::GetClosestVertex(const TransformComponent& transform, const Vec2f& inputPoint) const
{
	return _bounds->GetClosestVertex(transform, inputPoint);
}

float32 Collider2DComponent::GetMinX(const TransformComponent& transform) const
{
	return _bounds->GetMinX(transform);
}

float32 Collider2DComponent::GetMaxX(const TransformComponent& transform) const
{
	return _bounds->GetMaxX(transform);
}

Gizmo* Collider2DComponent::GetGizmo(const TransformComponent& transform) const
{
	return _bounds->GetGizmo(transform);
}
