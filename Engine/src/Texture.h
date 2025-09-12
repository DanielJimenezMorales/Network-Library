#pragma once
#include <SDL_image.h>

#include <cassert>

namespace Engine
{
	class Texture
	{
		public:
			Texture( SDL_Texture* texture, int32 width, int32 height )
			    : _texture( texture )
			    , _pixelWidth( width )
			    , _pixelHeight( height )
			{
				assert( _pixelWidth > 0 );
				assert( _pixelHeight > 0 );
			}

			int32 GetWidth() const { return _pixelWidth; }
			int32 GetHeight() const { return _pixelHeight; }
			SDL_Texture* GetRaw() const { return _texture; }

		private:
			SDL_Texture* _texture;
			int32 _pixelWidth;
			int32 _pixelHeight;
	};
} // namespace Engine
