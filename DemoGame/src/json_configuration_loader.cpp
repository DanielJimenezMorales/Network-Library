#include "json_configuration_loader.h"

#include <string>
#include <fstream>
#include "json.hpp"

#include "ecs/prefab.h"
#include "ecs/component_configuration.h"

#include "component_configurations/sprite_renderer_component_configuration.h"
#include "component_configurations/camera_component_configuration.h"

JsonConfigurationLoader::JsonConfigurationLoader()
    : IConfigurationLoader()
{
}

static void ParseComponentConfiguration( const nlohmann::json& json_data,
                                         ECS::ComponentConfiguration*& out_component_config )
{
	const std::string component_name = json_data[ "name" ];
	if ( component_name == "SpriteRenderer" )
	{
		const std::string texture_name = json_data[ "texture_path" ];
		out_component_config = new SpriteRendererComponentConfiguration( texture_name );
	}
	else if ( component_name == "Camera" )
	{
		const int32 width = json_data[ "width" ];
		const int32 height = json_data[ "height" ];
		out_component_config = new CameraComponentConfiguration( width, height );
	}
}

bool JsonConfigurationLoader::LoadPrefabs( std::vector< ECS::Prefab >& out_prefabs )
{
	static const std::string relative_path = "config_files/entity_prefabs/";
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

		ECS::Prefab prefab;

		nlohmann::json data = nlohmann::json::parse( input_stream );
		prefab.name.assign( data[ "name" ] );
		prefab.archetype.assign( data[ "archetype" ] );

		auto component_configs = data[ "component_configs" ];
		prefab.componentConfigurations.reserve( component_configs.size() );
		for ( auto components_cit = component_configs.cbegin(); components_cit != component_configs.cend();
		      ++components_cit )
		{
			ECS::ComponentConfiguration* component_config = nullptr;
			ParseComponentConfiguration( *components_cit, component_config );
			prefab.componentConfigurations[ component_config->name ] = component_config;
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
