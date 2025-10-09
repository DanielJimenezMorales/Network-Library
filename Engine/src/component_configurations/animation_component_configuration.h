#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

#include "components/animation_component.h"

#include <vector>
#include <string>

struct AnimationConfiguration
{
		std::string name;
		std::string path;
};

struct AnimationComponentConfiguration : public Engine::ECS::ComponentConfiguration
{
		AnimationComponentConfiguration( const std::vector< AnimationConfiguration >& animations,
		                                 const std::string& initial_animation_name )
		    : Engine::ECS::ComponentConfiguration( "Animation" )
		    , animations( animations )
		    , initialAnimationName( initial_animation_name )
		{
		}

		AnimationComponentConfiguration* Clone() const override { return new AnimationComponentConfiguration( *this ); }

		std::vector< AnimationConfiguration > animations;
		std::string initialAnimationName;
};
