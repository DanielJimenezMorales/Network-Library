#pragma once
#include "numeric_types.h"

#include "Vec2f.h"
#include "ecs/game_entity.hpp"

struct ShotEntry
{
		ShotEntry()
		    : position( 0.0f, 0.0f )
		    , direction( 0.0f, 1.0f )
		    , damage( 0 )
		    , shooterEntity()
		{
		}

		Vec2f position;
		Vec2f direction;
		uint32 damage;
		Engine::ECS::GameEntity shooterEntity;
};
