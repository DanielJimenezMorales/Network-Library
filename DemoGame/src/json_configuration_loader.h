#pragma once
#include "i_configuration_loader.h"

#include "json.hpp"
#include <string>
#include <fstream>

#include "ecs/archetype.h"

class JsonConfigurationLoader : public IConfigurationLoader
{
	public:
		JsonConfigurationLoader();

		bool LoadArchetypes( std::vector< Engine::ECS::Archetype >& out_archetypes ) override
		{
			static const std::string relative_path = "config_files/entity_archetypes/";
			std::vector< std::string > archetype_files;
			GetAllFilesInDirectory( archetype_files, relative_path );

			for ( auto cit = archetype_files.cbegin(); cit != archetype_files.cend(); ++cit )
			{
				const std::string file_path = relative_path + *cit;
				std::ifstream input_stream( file_path );
				if ( !input_stream.is_open() )
				{
					continue;
				}

				Engine::ECS::Archetype archetype;

				nlohmann::json data = nlohmann::json::parse( input_stream );
				archetype.name.assign( data[ "name" ] );

				auto json_components = data[ "components" ];
				archetype.components.reserve( json_components.size() );
				for ( auto components_cit = json_components.cbegin(); components_cit != json_components.cend();
				      ++components_cit )
				{
					archetype.components.push_back( *components_cit );
				}

				out_archetypes.push_back( std::move( archetype ) );
			}

			return true;
		};

		bool LoadPrefabs( std::vector< Engine::ECS::Prefab >& out_prefabs ) override;

	private:
		bool GetAllFilesInDirectory( std::vector< std::string >& out_files, const std::string& directory_name );
};