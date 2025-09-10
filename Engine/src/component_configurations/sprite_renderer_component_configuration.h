#pragma once
#include "ecs/component_configuration.h"

#include "components/sprite_renderer_component.h"

namespace Engine
{
	struct SpriteRendererComponentConfiguration : public ECS::ComponentConfiguration
	{
			SpriteRendererComponentConfiguration( const std::string& texture_path, Engine::SpriteType type );

			SpriteRendererComponentConfiguration* Clone() const override;

			std::string texturePath;
			Engine::SpriteType type;
	};
}
