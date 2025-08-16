#pragma once
#include "vec2f.h"

namespace Engine
{
	/// <summary>
	/// <para>Rotations:
	/// - Will be represented using a float32 for the rotation angle (0 - 360).
	/// - LEFT = 0 degrees / (1, 0), RIGHT = 180 degrees / (-1, 0), UP = 90 degrees / (0, 1), DOWN = 270 degrees / (0,
	/// -1)
	/// - The transform rotates anti-clockwise. That means that if the transform is looking left (1, 0) and we rotate it
	/// 90 degrees, it will end up looking up (0, 1)</para>
	/// </summary>
	struct TransformComponent
	{
		public:
			TransformComponent();
			TransformComponent( float32 x, float32 y );
			TransformComponent( const Vec2f& position, float32 rotation );

			TransformComponent( const TransformComponent& other ) = default;

			// Position
			Vec2f GetPosition() const { return _position; }
			void SetPosition( const Vec2f& newPosition ) { _position = newPosition; }

			// Rotation
			float32 GetRotationAngle() const { return _rotationAngle; }
			void SetRotationAngle( float32 newRotationAngle );
			void SetRotationLookAt( Vec2f look_at_direction );
			void LookAt( const Vec2f& position );
			Vec2f GetForwardVector() const;

			// Scale
			Vec2f GetScale() const { return _scale; }
			void SetScale( const Vec2f& newScale ) { _scale = newScale; }

		private:
			Vec2f _position;

			float32 _rotationAngle;
			static constexpr Vec2f DEFAULT_FORWARD_VECTOR = Vec2f( 0.f, -1.f );

			Vec2f _scale;
	};
} // namespace Engine
