#pragma once
#pragma once
#include "asset_manager/asset.h"

#include "numeric_types.h"

#include <string>
#include <variant>
#include <unordered_map>
#include <vector>

namespace Engine
{
	enum class ConfigurationValueType
	{
		FLOAT32 = 0,
		INT32 = 1,
		STRING = 2
	};

	struct ConfigurationValue
	{
			std::string name;
			std::variant< float32, int32, std::string > value;
			ConfigurationValueType type;
	};

	class ConfigurationAsset : public Asset
	{
		public:
			ConfigurationAsset( const std::string& path, std::vector< ConfigurationValue >&& values )
			    : Asset( path )
			    , _configurationValues()
			{
				for ( auto& value : values )
				{
					_configurationValues.emplace( value.name, std::move( value ) );
				}
			}

			AssetType GetType() const override { return AssetType::CONFIGURATION; }

			float32 GetFloat( const std::string& name ) const;
			int32 GetInt( const std::string& name ) const;
			const std::string& GetString( const std::string& name ) const;
			// TODO Consider adding another one for complex objects that also contains the GetFloat, GetInt and
			// GetString methods

		private:
			std::unordered_map< std::string, ConfigurationValue > _configurationValues;
	};
} // namespace Engine
