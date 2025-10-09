#pragma once
#include "numeric_types.h"

#include "asset_manager/asset_handle.h"

#include <string>
#include <unordered_map>

namespace Engine
{
	struct AnimationClip
	{
			AnimationClip()
			    : name()
			    , assetHandle( AssetHandle::GetInvalid() )
			{
			}

			AnimationClip( const std::string& name, const AssetHandle& asset_handle )
			    : name( name )
			    , assetHandle( asset_handle )
			{
			}

			std::string name;
			AssetHandle assetHandle;
	};

	struct AnimationComponent
	{
		public:
			AnimationComponent()
			    : animations()
			    , currentAnimation()
			    , currentFrame( 0 )
			    , timeAccumulator( 0.f )
			{
			}

			std::unordered_map< std::string, AnimationClip > animations;
			const AnimationClip* currentAnimation;

			uint32 currentFrame;
			float32 timeAccumulator;
	};
} // namespace Engine
