#pragma once
#include "asset_manager/asset.h"

#include "numeric_types.h"

#include <SDL_image.h>
#include <cassert>

namespace Engine
{
	class TextureAsset : public Asset
	{
		public:
			TextureAsset( const std::string& path, SDL_Texture* texture, int32 width, int32 height )
			    : Asset( path )
			    , _texture( texture )
			    , _pixelWidth( width )
			    , _pixelHeight( height )
			{
				assert( _pixelWidth > 0 );
				assert( _pixelHeight > 0 );
			}

			AssetType GetType() const override { return AssetType::TEXTURE; }

			int32 GetWidth() const { return _pixelWidth; }
			int32 GetHeight() const { return _pixelHeight; }
			SDL_Texture* GetRaw() const { return _texture; }

		private:
			SDL_Texture* _texture;
			int32 _pixelWidth;
			int32 _pixelHeight;
	};
} // namespace Engine
