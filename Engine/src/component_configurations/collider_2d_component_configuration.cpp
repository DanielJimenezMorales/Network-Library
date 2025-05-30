#include "collider_2d_component_configuration.h"

namespace Engine
{
	Collider2DComponentConfiguration::Collider2DComponentConfiguration()
	    : ECS::ComponentConfiguration( "Collider2D" )
	    , boundsConfiguration( nullptr )
	    , isTrigger( false )
	    , collisionResponseType( CollisionResponseType::Dynamic )
	{
	}

	Collider2DComponentConfiguration::Collider2DComponentConfiguration( Bounds2DConfiguration* bounds_config,
	                                                                    bool is_trigger,
	                                                                    CollisionResponseType collision_response_type )
	    : ECS::ComponentConfiguration( "Collider2D" )
	    , boundsConfiguration( bounds_config )
	    , isTrigger( is_trigger )
	    , collisionResponseType( collision_response_type )
	{
	}

	Collider2DComponentConfiguration::Collider2DComponentConfiguration( const Collider2DComponentConfiguration& other )
	    : ECS::ComponentConfiguration( "Collider2D" )
	    , boundsConfiguration( ( other.boundsConfiguration != nullptr ) ? other.boundsConfiguration->Clone() : nullptr )
	    , isTrigger( other.isTrigger )
	    , collisionResponseType( other.collisionResponseType )
	{
	}

	Collider2DComponentConfiguration::~Collider2DComponentConfiguration()
	{
		Free();
	}

	Collider2DComponentConfiguration& Collider2DComponentConfiguration::operator=(
	    const Collider2DComponentConfiguration& other )
	{
		if ( this == &other )
		{
			return *this;
		}

		Free();
		boundsConfiguration = ( other.boundsConfiguration != nullptr ) ? other.boundsConfiguration->Clone() : nullptr;
		isTrigger = other.isTrigger;
		collisionResponseType = other.collisionResponseType;
		return *this;
	}

	Collider2DComponentConfiguration* Collider2DComponentConfiguration::Clone() const
	{
		return new Collider2DComponentConfiguration( *this );
	}

	void Collider2DComponentConfiguration::Free()
	{
		if ( boundsConfiguration != nullptr )
		{
			delete boundsConfiguration;
			boundsConfiguration = nullptr;
		}
	}
} // namespace Engine
