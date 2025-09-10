#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct AnimationComponentConfiguration : public Engine::ECS::ComponentConfiguration
{
		AnimationComponentConfiguration( uint32 start_animation_x_pixel, uint32 start_animation_y_pixel,
		                                 uint32 frame_width_pixels, uint32 frame_height_pixels, uint32 number_of_frames,
		                                 uint32 frame_rate )
		    : Engine::ECS::ComponentConfiguration( "Animation" )
		    , startFrameXPixel( start_animation_x_pixel )
		    , startFrameYPixel( start_animation_y_pixel )
		    , frameWidthPixels( frame_width_pixels )
		    , frameHeightPixels( frame_height_pixels )
		    , numberOfFrames( number_of_frames )
		    , frameRate( frame_rate )
		{
		}

		AnimationComponentConfiguration* Clone() const override { return new AnimationComponentConfiguration( *this ); }

		uint32 startFrameXPixel;
		uint32 startFrameYPixel;
		uint32 frameWidthPixels;
		uint32 frameHeightPixels;
		uint32 numberOfFrames;
		uint32 frameRate;
};
