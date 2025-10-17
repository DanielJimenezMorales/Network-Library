#pragma once
#include "numeric_types.h"

#include <string>
#include <vector>

namespace Engine
{
	struct AnimationComponent;
	struct AnimationClip;
	class AssetManager;

	class AnimationComponentProxy
	{
		public:
			AnimationComponentProxy() = default;

			/// <summary>
			/// Adds an animation clip to the animation component. Returns True if added successfully, False otherwise.
			/// </summary>
			bool AddAnimationClip( AnimationComponent& anim, AnimationClip& clip );

			/// <summary>
			/// Returns the current frame of the animation being played.
			/// </summary>
			uint32 GetCurrentAnimationFrame( const AnimationComponent& anim ) const;

			/// <summary>
			/// True if any animation is playing, False otherwise.
			/// </summary>
			bool IsPlaying( const AnimationComponent& anim ) const;

			/// <summary>
			/// True if animation with name 'name' is playing, False otherwise.
			/// </summary>
			bool IsPlaying( const AnimationComponent& anim, const std::string& name ) const;

			/// <summary>
			/// Returns the animation it is currently being played, or nullptr if no animation is playing.
			/// </summary>
			const AnimationClip* GetCurrentAnimation( const AnimationComponent& anim ) const;

			/// <summary>
			/// Advances the current animation by elapsed_time seconds.
			/// </summary>
			void AdvanceAnimation( AnimationComponent& anim, float32 elapsed_time, const AssetManager& asset_manager );

			/// <summary>
			/// Plays the animation with name 'animation_name'. If that animation was already being played, it will be
			/// restarted.
			/// </summary>
			void PlayAnimation( AnimationComponent& anim, const std::string& animation_name );

			/// <summary>
			/// Plays the animation with name 'animation_name'. If that animation was already being played, this
			/// function does nothing.
			/// </summary>
			void PlayAnimationIfNotBeingPlayed( AnimationComponent& anim, const std::string& animation_name );

			/// <summary>
			/// Stops playing the current animation. If not animation was being played, this function does nothing.
			/// </summary>
			void StopAnimation( AnimationComponent& anim );

		private:
			void RestartCurrentAnimation( AnimationComponent& anim );
			void PlayAnimationInternal( AnimationComponent& anim, const std::string& animation_name );
	};
} // namespace Engine
