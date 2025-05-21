#include "sprite_renderer_component_configuration.h"

SpriteRendererComponentConfiguration::SpriteRendererComponentConfiguration( const std::string& texture_path )
    : ECS::ComponentConfiguration( "SpriteRenderer" )
    , texturePath( texture_path )
{
}

SpriteRendererComponentConfiguration* SpriteRendererComponentConfiguration::Clone() const
{
	return new SpriteRendererComponentConfiguration( *this );
}
