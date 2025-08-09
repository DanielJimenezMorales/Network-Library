#pragma once
#include "numeric_types.h"

#include <math.h>

struct Vec2f
{
	public:
		constexpr Vec2f()
		    : _x( 0.f )
		    , _y( 0.f )
		{
		}

		constexpr Vec2f( float32 x, float32 y )
		    : _x( x )
		    , _y( y )
		{
		}

		constexpr Vec2f( int32 x, int32 y )
		    : _x( static_cast< float32 >( x ) )
		    , _y( static_cast< float32 >( y ) )
		{
		}

		Vec2f( const Vec2f& other ) = default;

		static constexpr Vec2f Zero() { return Vec2f( 0.f, 0.f ); }

		static float32 GetSquareDistance( const Vec2f& v1, const Vec2f& v2 )
		{
			const float32 dx = v1.X() - v2.X();
			const float32 dy = v1.Y() - v2.Y();
			return ( dx * dx ) + ( dy * dy );
		}

		float32 X() const { return _x; }
		float32 Y() const { return _y; }

		void X( float32 newX ) { _x = newX; }
		void Y( float32 newY ) { _y = newY; }

		void AddToX( float32 newX ) { _x += newX; }
		void AddToY( float32 newY ) { _y += newY; }

		// +
		Vec2f operator+( const Vec2f& rhs ) const { return Vec2f( _x + rhs._x, _y + rhs._y ); }
		Vec2f operator+( float32 rhs ) const { return Vec2f( _x + rhs, _y + rhs ); }
		friend Vec2f operator+( float32 lhs, const Vec2f& rhs ) { return rhs + lhs; }

		// -
		Vec2f operator-( const Vec2f& rhs ) const { return Vec2f( _x - rhs._x, _y - rhs._y ); }
		Vec2f operator-( float32 rhs ) const { return Vec2f( _x - rhs, _y - rhs ); }

		// *
		Vec2f operator*( float32 rhs ) const { return Vec2f( _x * rhs, _y * rhs ); }
		friend Vec2f operator*( float32 lhs, const Vec2f& rhs ) { return rhs * lhs; }

		// /
		Vec2f operator/( float32 rhs ) const { return Vec2f( _x / rhs, _y / rhs ); }

		Vec2f& operator+=( float32 rhs )
		{
			_x += rhs;
			_y += rhs;
			return *this;
		}

		Vec2f& operator-=( float32 rhs )
		{
			_x -= rhs;
			_y -= rhs;
			return *this;
		}

		Vec2f& operator*=( float32 rhs )
		{
			_x *= rhs;
			_y *= rhs;
			return *this;
		}

		Vec2f& operator/=( float32 rhs )
		{
			_x /= rhs;
			_y /= rhs;
			return *this;
		}

		Vec2f& operator=( const Vec2f& rhs )
		{
			_x = rhs._x;
			_y = rhs._y;
			return *this;
		}

		bool operator==( const Vec2f& rhs ) const
		{
			if ( fabs( _x - rhs._x ) <= _EPSILON && fabs( _y - rhs._y ) <= _EPSILON )
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		float32 Dot( const Vec2f& other ) const { return ( _x * other._x ) + ( _y * other._y ); }

		float32 Magnitude() const { return sqrtf( SqMagnitude() ); }

		float32 SqMagnitude() const { return ( _x * _x ) + ( _y * _y ); }

		void Normalize()
		{
			// Check for this in order to avoid dividing by 0
			if ( fabs( _x ) < _EPSILON && fabs( _y ) < _EPSILON )
			{
				return;
			}

			*this /= Magnitude();
		}

		Vec2f GetNormalize() const { return Vec2f( *this ) /= Magnitude(); }

	private:
		float32 _x;
		float32 _y;

		static constexpr float32 _EPSILON = 1.0e-04f;
};
