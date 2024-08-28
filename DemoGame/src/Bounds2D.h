#pragma once
#include "Vec2f.h"
#include <vector>

struct TransformComponent;
class Gizmo;

enum class CollisionShapeType : uint8_t
{
	Convex = 0,
	Circle = 1,
};

class Bounds2D
{
public:
	virtual ~Bounds2D() {}

	CollisionShapeType GetShapeType() const { return _shapeType; }
	virtual void GetAxes(const TransformComponent& transform, std::vector<Vec2f>& outAxes) const = 0;
	virtual void ProjectAxis(const TransformComponent& transform, const Vec2f& axis, float& outMin, float& outMax) const = 0;
	virtual Vec2f GetClosestVertex(const TransformComponent& transform, const Vec2f& inputPoint) const = 0;

	virtual float GetMinX(const TransformComponent& transform) const = 0;
	virtual float GetMaxX(const TransformComponent& transform) const = 0;

	virtual Gizmo* GetGizmo(const TransformComponent& transform) const = 0;

protected:
	Bounds2D(CollisionShapeType shapeType) : _shapeType(shapeType)
	{
	}

private:
	CollisionShapeType _shapeType;
};
