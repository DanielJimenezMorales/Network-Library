#pragma once
#pragma once
#include "asset_manager/asset.h"

#include "numeric_types.h"
#include "asserts.h"

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

			template < typename T >
			const T& GetValue( const std::string& name ) const;

		private:
			std::unordered_map< std::string, ConfigurationValue > _configurationValues;
	};

	template < typename T >
	inline const T& ConfigurationAsset::GetValue( const std::string& name ) const
	{
		auto found = _configurationValues.find( name );
		ASSERT( found != _configurationValues.end(), "Configuration value with name: %s doesn't exist in %s",
		        name.c_str(), GetPath().c_str() );

		return std::get< T >( found->second.value );
	}
} // namespace Engine
