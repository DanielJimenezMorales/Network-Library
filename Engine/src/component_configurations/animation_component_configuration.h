#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

#include "components/animation_component.h"

#include <vector>
#include <string>

struct AnimationComponentConfiguration : public Engine::ECS::ComponentConfiguration
{
		AnimationComponentConfiguration( const std::vector< Engine::AnimationClip >& animations,
		                                 const std::string& initial_animation_name )
		    : Engine::ECS::ComponentConfiguration( "Animation" )
		    , animations( animations )
		    , initialAnimationName( initial_animation_name )
		{
		}

		AnimationComponentConfiguration* Clone() const override { return new AnimationComponentConfiguration( *this ); }

		std::vector< Engine::AnimationClip > animations;
		std::string initialAnimationName;
};
