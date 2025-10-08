#pragma once
#include "vec2f.h"

#include "asset_manager/asset_handle.h"

namespace Engine
{
	enum class SpriteType
	{
		SINGLE = 0,
		SPRITE_SHEET = 1
	};

	struct SpriteRendererComponent
	{
		public:
			SpriteRendererComponent()
			    : textureHandler( AssetHandle::GetInvalid() )
			    , type( SpriteType::SINGLE )
			    , width( 0 )
			    , height( 0 )
			    , uv0( 0.f, 0.f )
			    , uv1( 1.f, 1.f )
			    , flipX( false )
			    , flipY( false )
			{
			}

			SpriteRendererComponent( const AssetHandle& texture_handler )
			    : textureHandler( texture_handler )
			    , type( SpriteType::SINGLE )
			    , width( 0 )
			    , height( 0 )
			    , uv0( 0.f, 0.f )
			    , uv1( 1.f, 1.f )
			    , flipX( false )
			    , flipY( false )
			{
			}

			AssetHandle textureHandler;
			SpriteType type;
			uint32 width;
			uint32 height;
			Vec2f uv0;
			Vec2f uv1;
			bool flipX;
			bool flipY;
	};
} // namespace Engine
