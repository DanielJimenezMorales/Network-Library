#include "animation_component_proxy.h"
#include "logger.h"

#include "components/animation_component.h"

#include "asset_manager/asset_manager.h"

#include "animation/animation_asset.h"

#include <cassert>

namespace Engine
{
	bool AnimationComponentProxy::AddAnimationClip( AnimationComponent& anim, AnimationClip& clip )
	{
		bool result = false;
		if ( anim.animations.find( clip.name ) == anim.animations.end() )
		{
			anim.animations[ clip.name ] = clip;
			result = true;
		}
		else
		{
			LOG_ERROR( "[%s] Animation clip with name %s already exists.", THIS_FUNCTION_NAME, clip.name.c_str() );
		}

		return result;
	}

	uint32 AnimationComponentProxy::GetCurrentAnimationFrame( const AnimationComponent& anim ) const
	{
		return anim.currentFrame;
	}

	bool AnimationComponentProxy::IsPlaying( const AnimationComponent& anim ) const
	{
		return anim.currentAnimation != nullptr;
	}

	bool AnimationComponentProxy::IsPlaying( const AnimationComponent& anim, const std::string& name ) const
	{
		bool result = false;
		if ( anim.currentAnimation != nullptr )
		{
			if ( anim.currentAnimation->name == name )
			{
				result = true;
			}
		}

		return result;
	}

	const AnimationClip* AnimationComponentProxy::GetCurrentAnimation( const AnimationComponent& anim ) const
	{
		return IsPlaying( anim ) ? anim.currentAnimation : nullptr;
	}

	void AnimationComponentProxy::AdvanceAnimation( AnimationComponent& anim, float32 elapsed_time,
	                                                const AssetManager& asset_manager )
	{
		// Try get animation asset
		const AnimationAsset* currentAnimationAsset =
		    asset_manager.GetRawAsset< AnimationAsset >( anim.currentAnimation->assetHandle, AssetType::ANIMATION );
		assert( currentAnimationAsset != nullptr );

		anim.timeAccumulator += elapsed_time;
		const float32 frameDuration = 1.f / currentAnimationAsset->GetFrameRate();
		if ( anim.timeAccumulator >= frameDuration )
		{
			const uint32 numberOfFramesToAdvance = static_cast< uint32 >( anim.timeAccumulator / frameDuration );
			anim.currentFrame =
			    ( anim.currentFrame + numberOfFramesToAdvance ) % currentAnimationAsset->GetNumberOfFrames();
			anim.timeAccumulator -= numberOfFramesToAdvance * frameDuration;

			// If it is not loop then stop animation
			if ( anim.currentFrame == 0 && !currentAnimationAsset->IsLoop() )
			{
				StopAnimation( anim );
			}
		}
	}

	void AnimationComponentProxy::PlayAnimation( AnimationComponent& anim, const std::string& animation_name )
	{
		if ( !IsPlaying( anim, animation_name ) )
		{
			PlayAnimationInternal( anim, animation_name );
		}
		else
		{
			RestartCurrentAnimation( anim );
		}
	}

	void AnimationComponentProxy::PlayAnimationIfNotBeingPlayed( AnimationComponent& anim,
	                                                             const std::string& animation_name )
	{
		if ( !IsPlaying( anim, animation_name ) )
		{
			PlayAnimationInternal( anim, animation_name );
		}
	}

	void AnimationComponentProxy::StopAnimation( AnimationComponent& anim )
	{
		if ( IsPlaying( anim ) )
		{
			RestartCurrentAnimation( anim );
			anim.currentAnimation = nullptr;
		}
	}

	void AnimationComponentProxy::RestartCurrentAnimation( AnimationComponent& anim )
	{
		anim.currentFrame = 0;
		anim.timeAccumulator = 0.f;
	}

	void AnimationComponentProxy::PlayAnimationInternal( AnimationComponent& anim, const std::string& animation_name )
	{
		if ( anim.animations.find( animation_name ) != anim.animations.end() )
		{
			const Engine::AnimationClip* newAnimationClip = &anim.animations[ animation_name ];
			anim.currentAnimation = newAnimationClip;
			RestartCurrentAnimation( anim );
		}
		else
		{
			LOG_ERROR( "[%s] Animation Clip with name %s couldn't be found", THIS_FUNCTION_NAME,
			           animation_name.c_str() );
		}
	}
} // namespace Engine
