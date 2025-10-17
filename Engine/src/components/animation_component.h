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

	// TODO Create a proxy class for this component as I did for the transform component to avoid direct manipulation of
	// the struct data. Multiple systems are going to need to touch it and we should provide a secure wrapper for
	// manipulating the component
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
