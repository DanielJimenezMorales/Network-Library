#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct AnimationComponentConfiguration : public Engine::ECS::ComponentConfiguration
{
		AnimationComponentConfiguration( uint32 pixels_per_frame, uint32 number_of_frames, uint32 frame_rate )
		    : Engine::ECS::ComponentConfiguration( "Animation" )
		    , pixelsPerFrame( pixels_per_frame )
		    , numberOfFrames( number_of_frames )
		    , frameRate( frame_rate )
		{
		}

		AnimationComponentConfiguration* Clone() const override { return new AnimationComponentConfiguration( *this ); }

		uint32 pixelsPerFrame;
		uint32 numberOfFrames;
		uint32 frameRate;
};
