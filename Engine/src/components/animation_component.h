#pragma once
#include "numeric_types.h"

namespace Engine
{
	struct AnimationComponent
	{
		public:
			uint32 startFrameXPixel;
			uint32 startFrameYPixel;
			uint32 frameWidthPixels;
			uint32 frameHeightPixels;
			uint32 numberOfFrames;
			uint32 frameRate;
			uint32 currentFrame;
			float32 timeAccumulator;
	};
}
