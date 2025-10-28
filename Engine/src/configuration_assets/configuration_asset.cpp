#include "configuration_asset.h"

#include "asserts.h"

namespace Engine
{
	float32 ConfigurationAsset::GetFloat( const std::string& name ) const
	{
		auto found = _configurationValues.find( name );
		ASSERT( found != _configurationValues.end(), "Configuration value with name: %s doesn't exist in %s",
		        name.c_str(), GetPath().c_str() );
		ASSERT( found->second.type == ConfigurationValueType::FLOAT32,
		        "Configuration value from %s with name: %s is not a FLOAT32", GetPath().c_str(), name.c_str() );
		return std::get< float32 >( found->second.value );
	}

	int32 ConfigurationAsset::GetInt( const std::string& name ) const
	{
		auto found = _configurationValues.find( name );
		ASSERT( found != _configurationValues.end(), "Configuration value with name: %s doesn't exist in %s",
		        name.c_str(), GetPath().c_str() );
		ASSERT( found->second.type == ConfigurationValueType::INT32,
		        "Configuration value from %s with name: %s is not a INT32", GetPath().c_str(), name.c_str() );
		return std::get< int32 >( found->second.value );
	}

	const std::string& ConfigurationAsset::GetString( const std::string& name ) const
	{
		auto found = _configurationValues.find( name );
		ASSERT( found != _configurationValues.end(), "Configuration value with name: %s doesn't exist in %s",
		        name.c_str(), GetPath().c_str() );
		ASSERT( found->second.type == ConfigurationValueType::STRING,
		        "Configuration value from %s with name: %s is not a STRING", GetPath().c_str(), name.c_str() );
		return std::get< std::string >( found->second.value );
	}
} // namespace Engine
