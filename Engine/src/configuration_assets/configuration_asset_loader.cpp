#include "configuration_asset_loader.h"

#include "configuration_assets/configuration_asset.h"

#include "logger.h"
#include "json.hpp"

#include <fstream>
#include <filesystem>
#include <vector>

namespace Engine
{
	Asset* ConfigurationAssetLoader::Load( const std::string& path )
	{
		Asset* result = nullptr;

		const std::filesystem::path basePath = std::filesystem::current_path();
		const std::filesystem::path filePath = basePath / path;

		if ( std::filesystem::exists( filePath ) )
		{
			std::ifstream inputStream( filePath );
			if ( inputStream.is_open() )
			{
				nlohmann::json data = nlohmann::json::parse( inputStream );

				std::vector< ConfigurationValue > values;
				auto jsonValues = data[ "values" ];
				values.reserve( jsonValues.size() );
				for ( auto valuesCit = jsonValues.cbegin(); valuesCit != jsonValues.cend(); ++valuesCit )
				{
					ConfigurationValue configValue;
					const std::string typeString( ( *valuesCit )[ "type" ] );

					if ( typeString == "FLOAT32" )
					{
						configValue.type = ConfigurationValueType::FLOAT32;
						configValue.value.emplace< float32 >( ( *valuesCit )[ "value" ] );
					}
					else if ( typeString == "INT32" )
					{
						configValue.type = ConfigurationValueType::INT32;
						configValue.value.emplace< int32 >( ( *valuesCit )[ "value" ] );
					}
					else if ( typeString == "STRING" )
					{
						configValue.type = ConfigurationValueType::STRING;
						configValue.value.emplace< std::string >( ( *valuesCit )[ "value" ] );
					}
					else
					{
						LOG_ERROR( "Can't read configuration value of type %s because it is not supported." );
					}

					configValue.name = ( *valuesCit )[ "name" ];

					values.push_back( std::move( configValue ) );
				}

				ConfigurationAsset* configurationAsset = new ConfigurationAsset( path, std::move( values ) );
				result = configurationAsset;
			}
			else
			{
				LOG_ERROR( "Can't open configuration asset file at path %s", path.c_str() );
			}
		}
		else
		{
			LOG_ERROR( "Configuration asset file at path %s doesn't exist", path.c_str() );
		}

		return result;
	}
} // namespace Engine
