#pragma once
#include "Vec2f.h"
#include "Bounds2D.h"
#include <cstdint>
#include <vector>

struct TransformComponent;
class Gizmo;

enum class CollisionResponseType : uint8_t
{
	Static = 0,
	Dynamic = 1
};

struct Collider2DComponent
{
public:
	Collider2DComponent(Bounds2D* bounds, bool isTrigger, CollisionResponseType responseType) : _bounds(bounds), _isTrigger(isTrigger), _collisionResponseType(responseType)
	{
	}

	~Collider2DComponent()
	{
		if (_bounds != nullptr)
		{
			delete(_bounds);
		}
	}

	CollisionShapeType GetShapeType() const { return _bounds->GetShapeType(); }
	void GetAxes(const TransformComponent& transform, std::vector<Vec2f>& outAxes) const;
	void ProjectAxis(const TransformComponent& transform, const Vec2f& axis, float& outMin, float& outMax) const;
	Vec2f GetClosestVertex(const TransformComponent& transform, const Vec2f& inputPoint) const;

	float GetMinX(const TransformComponent& transform) const;
	float GetMaxX(const TransformComponent& transform) const;

	Gizmo* GetGizmo(const TransformComponent& transform) const;

	CollisionResponseType GetCollisionResponse() const { return _collisionResponseType; }
	bool IsTrigger() const { return _isTrigger; }

private:
	Bounds2D* _bounds;
	bool _isTrigger;
	CollisionResponseType _collisionResponseType;
};
