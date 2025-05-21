#pragma once
#include "numeric_types.h"

struct RaycastComponent
{
		RaycastComponent()
		    : distance( 100.f )
		{
		}

		float32 distance;
};
