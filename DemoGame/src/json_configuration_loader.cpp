#include "json_configuration_loader.h"

#include <string>
#include <fstream>
#include "json.hpp"

JsonConfigurationLoader::JsonConfigurationLoader()
    : IConfigurationLoader()
{
}

bool JsonConfigurationLoader::LoadPrefabs( std::vector< ECS::Prefab >& out_prefabs )
{
	return false;
}

bool JsonConfigurationLoader::GetAllFilesInDirectory( std::vector< std::string >& out_files )
{
	for ( const auto& entry : std::filesystem::directory_iterator( "config_files/entity_archetypes" ) )
	{
		const std::string filename = entry.path().filename().string();
		if ( filename.find( ".json" ) )
		{
			out_files.push_back( entry.path().filename().string() );
		}
	}
	return false;
}
