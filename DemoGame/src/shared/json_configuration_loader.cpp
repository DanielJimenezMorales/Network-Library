#include "json_configuration_loader.h"

#include "json.hpp"

#include "ecs/prefab.h"
#include "ecs/component_configuration.h"

#include "component_configurations/sprite_renderer_component_configuration.h"
#include "component_configurations/camera_component_configuration.h"
#include "component_configurations/collider_2d_component_configuration.h"
#include "component_configurations/animation_component_configuration.h"

#include "shared/component_configurations/player_controller_component_configuration.h"
#include "shared/component_configurations/temporary_lifetime_component_configuration.h"
#include "shared/component_configurations/health_component_configuration.h"

#include "collisions/circle_bounds_2d.h"
#include "collisions/collision_utils.h"

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
		out_component_config = new Engine::SpriteRendererComponentConfiguration( texture_name );
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
		const uint32 movement_speed = json_data[ "movement_speed" ];
		const uint32 fire_rate_per_second = json_data[ "fire_rate_per_second" ];
		out_component_config = new PlayerControllerComponentConfiguration( movement_speed, fire_rate_per_second );
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
		const uint32 startFrameXPixel = json_data[ "start_frame_x_pixel" ];
		const uint32 startFrameYPixel = json_data[ "start_frame_y_pixel" ];
		const uint32 frameWidthPixels = json_data[ "frame_width_pixels" ];
		const uint32 frameHeightPixels = json_data[ "frame_height_pixels" ];
		const uint32 numberOfFrames = json_data[ "number_of_frames" ];
		const uint32 frameRate = json_data[ "frame_rate" ];
		out_component_config = new AnimationComponentConfiguration(
		    startFrameXPixel, startFrameYPixel, frameWidthPixels, frameHeightPixels, numberOfFrames, frameRate );
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
				assert( component_config != nullptr );
				prefab.componentConfigurations[ component_config->name ] = component_config;
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
