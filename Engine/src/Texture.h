#pragma once
#include <SDL_image.h>

namespace Engine
{
	class Texture
	{
		public:
			Texture( SDL_Texture* texture, const SDL_Rect& dimensions )
			    : _texture( texture )
			    , _texturePixelDimensions( dimensions )
			{
			}

			const SDL_Rect& GetDimensions() const { return _texturePixelDimensions; }
			SDL_Texture* GetRaw() const { return _texture; }

		private:
			SDL_Texture* _texture;
			SDL_Rect _texturePixelDimensions;
	};
}
