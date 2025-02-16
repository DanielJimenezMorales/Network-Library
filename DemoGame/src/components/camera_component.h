#pragma once
#include "Vec2f.h"

struct CameraComponent
{
		static constexpr uint32 PIXELS_PER_WORLD_UNIT = 10;

		CameraComponent()
		    : width( 0 )
		    , height( 0 )
		{
		}

		CameraComponent( int32 width, int32 height )
		    : width( width )
		    , height( height )
		{
		}

		int32 width;
		int32 height;
};