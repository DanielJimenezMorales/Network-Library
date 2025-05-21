#pragma once
#include "ecs/component_configuration.h"

#include "components/collider_2d_component.h"
#include "collisions/Bounds2D.h"

namespace Engine
{
	struct Collider2DComponentConfiguration : ECS::ComponentConfiguration
	{
			Collider2DComponentConfiguration();
			Collider2DComponentConfiguration( Bounds2DConfiguration* bounds_config, bool is_trigger,
			                                  CollisionResponseType collision_response_type );
			Collider2DComponentConfiguration( const Collider2DComponentConfiguration& other );
			Collider2DComponentConfiguration( Collider2DComponentConfiguration&& other ) noexcept = default;

			Collider2DComponentConfiguration& operator=( const Collider2DComponentConfiguration& other );
			Collider2DComponentConfiguration& operator=( Collider2DComponentConfiguration&& other ) noexcept = default;

			~Collider2DComponentConfiguration();

			Collider2DComponentConfiguration* Clone() const override;

			Bounds2DConfiguration* boundsConfiguration;
			bool isTrigger;
			CollisionResponseType collisionResponseType;

		private:
			void Free();
	};
}
