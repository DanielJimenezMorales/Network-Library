#pragma once
#include "numeric_types.h"

namespace Engine
{
	struct AnimationComponent
	{
		public:
			uint32 pixelsPerFrame;
			uint32 numberOfFrames;
			uint32 frameRate;
			uint32 currentFrame;
			float32 timeAccumulator;
	};
}
