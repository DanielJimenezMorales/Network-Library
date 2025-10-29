#pragma once
#include "numeric_types.h"

namespace Engine
{
	struct Color
	{
		public:
			static Color RED() { return Color( 255, 0, 0, 255 ); }
			static Color GREEN() { return Color( 0, 255, 0, 255 ); }
			static Color BLUE() { return Color( 0, 0, 255, 255 ); }
			static Color WHITE() { return Color( 255, 255, 255, 255 ); }
			static Color BLACK() { return Color( 0, 0, 0, 255 ); }

			Color()
			    : r( 0 )
			    , g( 0 )
			    , b( 0 )
			    , a( 255 )
			{
			}

			Color( uint8 red, uint8 green, uint8 blue, uint8 alpha )
			    : r( red )
			    , g( green )
			    , b( blue )
			    , a( alpha )
			{
			}

			uint8 R() const { return r; }
			uint8 G() const { return g; }
			uint8 B() const { return b; }
			uint8 A() const { return a; }

		private:
			uint8 r;
			uint8 g;
			uint8 b;
			uint8 a;
	};
} // namespace Engine