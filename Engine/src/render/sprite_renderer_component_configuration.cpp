#include "sprite_renderer_component_configuration.h"

namespace Engine
{
	SpriteRendererComponentConfiguration::SpriteRendererComponentConfiguration( const std::string& texture_path,
	                                                                            Engine::SpriteType type )
	    : ECS::ComponentConfiguration( "SpriteRenderer" )
	    , texturePath( texture_path )
	    , type( type )
	{
	}

	SpriteRendererComponentConfiguration* SpriteRendererComponentConfiguration::Clone() const
	{
		return new SpriteRendererComponentConfiguration( *this );
	}
}
