#pragma once
#include "Vec2f.h"
#include "numeric_types.h"

#include "ecs/game_entity.hpp"

#include <vector>

struct TransformComponent;
class CircleBounds2D;

struct Ray
{
		Vec2f origin;
		Vec2f direction;
		uint32 maxDistance;
};

struct RaycastResult
{
		ECS::GameEntity entity;
		Vec2f position;
		Vec2f normal;
		uint32 squaredDistance;
};

bool PerformRaycastAgainstSphere( const Ray& ray, const TransformComponent& circle_transform,
                                  const CircleBounds2D& circle_collider, RaycastResult& out_result );
RaycastResult ExecuteRaycast( const Ray& ray, const std::vector< ECS::GameEntity >& entities_with_colliders );
