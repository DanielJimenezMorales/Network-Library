#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

namespace Engine
{
	struct CameraComponentConfiguration : public ECS::ComponentConfiguration
	{
			CameraComponentConfiguration( int32 width, int32 height )
			    : ECS::ComponentConfiguration( "Camera" )
			    , width( width )
			    , height( height )
			{
			}

			CameraComponentConfiguration* Clone() const override { return new CameraComponentConfiguration( *this ); }

			int32 width;
			int32 height;
	};
}
