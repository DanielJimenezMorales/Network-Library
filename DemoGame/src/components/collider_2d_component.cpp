#include "collider_2d_component.h"

#include "logger.h"

CollisionResponseType GetCollisionResponseTypeFromName( const std::string& name )
{
	if ( name == "Static" )
	{
		return CollisionResponseType::Static;
	}
	else if ( name == "Dynamic" )
	{
		return CollisionResponseType::Dynamic;
	}
	else
	{
		LOG_ERROR( "[GetCollisionResponseTypeFromName] : Can't parse %s to collision response type", name.c_str() );
	}

	return CollisionResponseType::None;
}
