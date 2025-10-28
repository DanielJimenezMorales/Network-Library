#include "json_configuration_loader.h"

#include "json.hpp"

#include "logger.h"

#include "camera_component_configuration.h"

#include "ecs/prefab.h"
#include "ecs/component_configuration.h"

#include "render/sprite_renderer_component_configuration.h"

#include "physics/collider_2d_component_configuration.h"
#include "physics/circle_bounds_2d.h"
#include "physics/collision_utils.h"

#include "animation/animation_component_configuration.h"

#include "shared/component_configurations/player_controller_component_configuration.h"
#include "shared/component_configurations/temporary_lifetime_component_configuration.h"
#include "shared/component_configurations/health_component_configuration.h"

#include <string>
#include <fstream>

JsonConfigurationLoader::JsonConfigurationLoader()
    : IConfigurationLoader()
{
}

static void ParseComponentConfiguration( const nlohmann::json& json_data,
                                         Engine::ECS::ComponentConfiguration*& out_component_config )
{
	const std::string component_name = json_data[ "name" ];
	if ( component_name == "SpriteRenderer" )
	{
		const std::string texture_name = json_data[ "texture_path" ];
		const std::string typeName = json_data[ "type" ];
		Engine::SpriteType type;
		if ( typeName == "SPRITE_SHEET" )
		{
			type = Engine::SpriteType::SPRITE_SHEET;
		}
		else
		{
			type = Engine::SpriteType::SINGLE;
		}

		out_component_config = new Engine::SpriteRendererComponentConfiguration( texture_name, type );
	}
	else if ( component_name == "Camera" )
	{
		const int32 width = json_data[ "width" ];
		const int32 height = json_data[ "height" ];
		out_component_config = new Engine::CameraComponentConfiguration( width, height );
	}
	else if ( component_name == "Collider2D" )
	{
		const bool is_trigger = json_data.value( "is_trigger", false );
		const std::string collision_response_type_name = json_data.value( "collision_response_type", "Static" );
		const Engine::CollisionResponseType collision_response_type =
		    Engine::GetCollisionResponseTypeFromName( collision_response_type_name );

		const nlohmann::json bounds_json_data = json_data[ "bounds_config" ];
		const std::string bounds_type = bounds_json_data[ "type" ];
		Engine::Bounds2DConfiguration* bounds_config = nullptr;

		if ( bounds_type == "Circle" )
		{
			const float32 radius = bounds_json_data[ "radius" ];
			bounds_config = new Engine::CircleBounds2DConfiguration( radius );
		}

		out_component_config =
		    new Engine::Collider2DComponentConfiguration( bounds_config, is_trigger, collision_response_type );
	}
	else if ( component_name == "PlayerController" )
	{
		out_component_config = new PlayerControllerComponentConfiguration();
	}
	else if ( component_name == "TemporaryLifetime" )
	{
		const float32 lifetime = json_data[ "lifetime" ];
		out_component_config = new TemporaryLifetimeComponentConfiguration( lifetime );
	}
	else if ( component_name == "Health" )
	{
		const uint32 max_health = json_data[ "max_health" ];
		const uint32 current_health =
		    ( json_data.contains( "current_health" ) ) ? json_data[ "current_health" ] : max_health;
		out_component_config = new HealthComponentConfiguration( max_health, current_health );
	}
	else if ( component_name == "Animation" )
	{
		std::string initialAnimationName = json_data[ "initial_animation_name" ];
		bool foundInitialAnimationName = false;
		std::vector< AnimationConfiguration > animations;
		auto animations_data = json_data[ "animations" ];
		auto cit = animations_data.cbegin();
		for ( ; cit != animations_data.cend(); ++cit )
		{
			const nlohmann::json& animation_json_data = *cit;

			AnimationConfiguration clip;
			clip.name = animation_json_data[ "name" ];
			clip.path = animation_json_data[ "path" ];
			animations.push_back( clip );

			if ( !foundInitialAnimationName )
			{
				if ( clip.name == initialAnimationName )
				{
					foundInitialAnimationName = true;
				}
			}
		}

		assert( !animations.empty() );

		if ( !foundInitialAnimationName )
		{
			LOG_WARNING( "Couldn't find Initial animation name %s. Setting the first animation name in the array",
			             initialAnimationName.c_str() );
			initialAnimationName = animations.front().name;
		}

		out_component_config = new AnimationComponentConfiguration( animations, initialAnimationName );
	}
}

bool JsonConfigurationLoader::LoadPrefabs( std::vector< Engine::ECS::Prefab >& out_prefabs,
                                           const std::string& relative_path )
{
	std::vector< std::string > prefab_files;
	GetAllFilesInDirectory( prefab_files, relative_path );

	for ( auto cit = prefab_files.cbegin(); cit != prefab_files.cend(); ++cit )
	{
		const std::string file_path = relative_path + *cit;
		std::ifstream input_stream( file_path );
		if ( !input_stream.is_open() )
		{
			continue;
		}

		Engine::ECS::Prefab prefab;

		nlohmann::json data = nlohmann::json::parse( input_stream );
		prefab.name.assign( data[ "name" ] );
		prefab.archetype.assign( data[ "archetype" ] );

		if ( data.contains( "component_configs" ) )
		{
			auto component_configs = data[ "component_configs" ];
			prefab.componentConfigurations.reserve( component_configs.size() );
			for ( auto components_cit = component_configs.cbegin(); components_cit != component_configs.cend();
			      ++components_cit )
			{
				Engine::ECS::ComponentConfiguration* component_config = nullptr;
				ParseComponentConfiguration( *components_cit, component_config );
				if ( component_config != nullptr )
				{
					prefab.componentConfigurations[ component_config->name ] = component_config;
				}
				else
				{
					LOG_ERROR( "[%s] Failed to parse component configuration with name %s in prefab %s",
					           THIS_FUNCTION_NAME, ( *components_cit )[ "name" ].get< std::string >().c_str(),
					           prefab.name.c_str() );
				}
			}
		}

		if ( data.contains( "children_prefabs" ) )
		{
			auto childrenPrefabs = data[ "children_prefabs" ];
			prefab.childrenPrefabs.reserve( childrenPrefabs.size() );
			for ( auto childrenPrefabsCit = childrenPrefabs.cbegin(); childrenPrefabsCit != childrenPrefabs.cend();
			      ++childrenPrefabsCit )
			{
				Engine::ECS::ChildPrefab childPrefab;
				childPrefab.name = ( *childrenPrefabsCit )[ "name" ];
				childPrefab.localPosition.X( ( *childrenPrefabsCit )[ "local_position_x" ] );
				childPrefab.localPosition.Y( ( *childrenPrefabsCit )[ "local_position_y" ] );
				childPrefab.localRotation = ( *childrenPrefabsCit )[ "local_rotation" ];
				childPrefab.localScale.X( ( *childrenPrefabsCit )[ "local_scale_x" ] );
				childPrefab.localScale.Y( ( *childrenPrefabsCit )[ "local_scale_y" ] );

				prefab.childrenPrefabs.push_back( childPrefab );
			}
		}

		out_prefabs.push_back( std::move( prefab ) );
	}

	return true;
}

bool JsonConfigurationLoader::GetAllFilesInDirectory( std::vector< std::string >& out_files,
                                                      const std::string& directory_name )
{
	for ( const auto& entry : std::filesystem::directory_iterator( directory_name.c_str() ) )
	{
		const std::string filename = entry.path().filename().string();
		if ( filename.find( ".json" ) )
		{
			out_files.push_back( entry.path().filename().string() );
		}
	}
	return false;
}
