#pragma once
#include "Vec2f.h"
#include "numeric_types.h"

#include "ecs/game_entity.hpp"

#include <vector>

namespace Raycaster
{
	struct Ray
	{
			Vec2f origin;
			Vec2f direction;
			uint32 maxDistance;
	};

	struct RaycastResult
	{
			RaycastResult()
			    : entity( ECS::GameEntity() )
			    , position()
			    , normal()
			    , squaredDistance( 0 )
			{
			}

			ECS::GameEntity entity;
			Vec2f position;
			Vec2f normal;
			uint32 squaredDistance;
	};

	RaycastResult ExecuteRaycast( const Ray& ray, const std::vector< ECS::GameEntity >& entities_with_colliders,
	                              const ECS::GameEntity& entity_to_exclude );
}
