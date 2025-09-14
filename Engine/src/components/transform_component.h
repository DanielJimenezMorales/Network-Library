#pragma once
#include "vec2f.h"

#include <vector>

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
			TransformComponent()
			    : _position( Vec2f( 0.f, 0.f ) )
			    , _rotationAngle( 0.f )
			    , _scale( Vec2f( 1.f, 1.f ) )
			{
			}

			TransformComponent( float32 x, float32 y )
			    : _position( Vec2f( x, y ) )
			    , _rotationAngle( 0.f )
			    , _scale( Vec2f( 1.f, 1.f ) )
			{
			}

			TransformComponent( const Vec2f& position, float32 rotation )
			    : _position( position )
			    , _rotationAngle( rotation )
			    , _scale( Vec2f( 1.f, 1.f ) )
			{
			}

			TransformComponent( const TransformComponent& other ) = default;
			TransformComponent& operator=( const TransformComponent& other ) = default;

		private:
			Vec2f _position;
			float32 _rotationAngle;
			Vec2f _scale;

			static constexpr Vec2f DEFAULT_FORWARD_VECTOR = Vec2f( 0.f, -1.f );

			friend class TransformComponentProxy;
			friend class ReadOnlyTransformComponentProxy;
	};
} // namespace Engine
