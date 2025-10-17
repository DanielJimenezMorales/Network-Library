#pragma once
#include "asset_manager/asset.h"

#include "numeric_types.h"

#include <cassert>

namespace Engine
{
	class AnimationAsset : public Asset
	{
		public:
			AnimationAsset( const std::string& path, const std::string& sprite_sheet_path, uint32 start_x,
			                uint32 start_y, uint32 frame_width, uint32 frame_height, uint32 number_of_frames,
			                uint32 frame_rate, bool flip_x )
			    : Asset( path )
			    , spriteSheetPath( sprite_sheet_path )
			    , startX( start_x )
			    , startY( start_y )
			    , frameWidth( frame_width )
			    , frameHeight( frame_height )
			    , numberOfFrames( number_of_frames )
			    , frameRate( frame_rate )
			    , flipX( flip_x )
			    , isLoop( true )
			{
				assert( frame_width > 0 );
				assert( frame_height > 0 );
				assert( number_of_frames > 0 );
				assert( frame_rate > 0 );
			}

			AssetType GetType() const override { return AssetType::ANIMATION; }

			const std::string& GetSpriteSheetPath() const { return spriteSheetPath; }
			uint32 GetStartX() const { return startX; }
			uint32 GetStartY() const { return startY; }
			uint32 GetFrameWidth() const { return frameWidth; }
			uint32 GetFrameHeight() const { return frameHeight; }
			uint32 GetNumberOfFrames() const { return numberOfFrames; }
			uint32 GetFrameRate() const { return frameRate; }
			bool IsFlippedX() const { return flipX; }
			bool IsLoop() const { return isLoop; }

		private:
			std::string spriteSheetPath;
			uint32 startX;
			uint32 startY;
			uint32 frameWidth;
			uint32 frameHeight;
			uint32 numberOfFrames;
			uint32 frameRate;
			bool flipX;
			bool isLoop;
	};
} // namespace Engine
