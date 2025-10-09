#include "animation_asset_loader.h"

#include "logger.h"

#include "animation/animation_asset.h"

#include "json.hpp"

#include <fstream>
#include <filesystem>

namespace Engine
{
	Asset* AnimationAssetLoader::Load( const std::string& path )
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

				const uint32 startX = data[ "start_x_pixel" ];
				const uint32 startY = data[ "start_y_pixel" ];
				const uint32 frameWidth = data[ "frame_width_pixels" ];
				const uint32 frameHeight = data[ "frame_height_pixels" ];
				const uint32 numberOfFrames = data[ "number_of_frames" ];
				const uint32 frameRate = data[ "frame_rate" ];
				const bool flipX = data[ "flip_x" ];

				AnimationAsset* animationAsset = new AnimationAsset( path, startX, startY, frameWidth, frameHeight,
				                                                     numberOfFrames, frameRate, flipX );
				result = animationAsset;
			}
			else
			{
				LOG_ERROR( "Couldn't open animation asset file at path %s", path.c_str() );
			}
		}
		else
		{
			LOG_ERROR( "Animation asset file at path %s doesn't exist", path.c_str() );
		}

		return result;
	}
} // namespace Engine
