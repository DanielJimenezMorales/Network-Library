#pragma once
#include "numeric_types.h"

namespace Engine
{
	struct RaycastComponent
	{
			RaycastComponent()
			    : distance( 100.f )
			{
			}

			float32 distance;
	};
}
