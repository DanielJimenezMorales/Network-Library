#pragma once
#include "numeric_types.h"

#include <string>
#include <unordered_map>

namespace Engine
{
	struct AnimationClip
	{
			std::string name;
			uint32 startFrameXPixel;
			uint32 startFrameYPixel;
			uint32 frameWidthPixels;
			uint32 frameHeightPixels;
			uint32 numberOfFrames;
			uint32 frameRate;
			bool flipX;
	};

	struct AnimationComponent
	{
		public:
			AnimationComponent()
			    : animations()
			    , currentAnimation( nullptr )
			    , currentFrame( 0 )
			    , timeAccumulator( 0.f )
			{
			}

			std::unordered_map< std::string, AnimationClip > animations;
			AnimationClip* currentAnimation;

			uint32 currentFrame;
			float32 timeAccumulator;
	};
} // namespace Engine
