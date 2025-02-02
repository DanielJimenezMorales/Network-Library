#pragma once
#include "ecs/component_configuration.h"

struct SpriteRendererComponentConfiguration : public ECS::ComponentConfiguration
{
		SpriteRendererComponentConfiguration( const std::string& texture_path );

		SpriteRendererComponentConfiguration* Clone() const override;

		std::string texturePath;
};
