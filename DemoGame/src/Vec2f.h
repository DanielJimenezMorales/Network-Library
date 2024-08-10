#pragma once
#include <math.h>

struct Vec2f
{
public:
	constexpr Vec2f() :_x(0.f), _y(0.f) {};
	constexpr Vec2f(float x, float y) : _x(x), _y(y) {}
	Vec2f(const Vec2f& other) = default;

	float X() const { return _x; }
	float Y() const { return _y; }

	void X(float newX) { _x = newX; }
	void Y(float newY) { _y = newY; }

	void AddToX(float newX) { _x += newX; }
	void AddToY(float newY) { _y += newY; }

	// +
	Vec2f operator+(const Vec2f& rhs) const { return Vec2f(_x + rhs._x, _y + rhs._y); }
	Vec2f operator+(float rhs) const { return Vec2f(_x + rhs, _y + rhs); }
	friend Vec2f operator+(float lhs, const Vec2f& rhs) { return rhs + lhs; }

	// -
	Vec2f operator-(const Vec2f& rhs) const{ return Vec2f(_x - rhs._x, _y - rhs._y); }
	Vec2f operator-(float rhs) const{ return Vec2f(_x - rhs, _y - rhs); }

	// *
	Vec2f operator*(float rhs) const{ return Vec2f(_x * rhs, _y * rhs); }
	friend Vec2f operator*(float lhs, const Vec2f& rhs) { return rhs * lhs; }

	// /
	Vec2f operator/(float rhs) const { return Vec2f(_x / rhs, _y / rhs); }

	Vec2f& operator+=(float rhs) { _x += rhs; _y += rhs; return *this; }

	Vec2f& operator-=(float rhs){ _x -= rhs; _y -= rhs; return *this; }

	Vec2f& operator*=(float rhs) { _x *= rhs; _y *= rhs; return *this; }

	Vec2f& operator/=(float rhs) { _x /= rhs; _y /= rhs; return *this; }

	Vec2f& operator=(const Vec2f& rhs) { _x = rhs._x; _y = rhs._y; return *this; }

	bool operator==(const Vec2f& rhs) const
	{
		if (fabs(_x - rhs._x) <= _EPSILON && fabs(_y - rhs._y) <= _EPSILON)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	float Dot(const Vec2f& other) const
	{
		return (_x * other._x) + (_y * other._y);
	}

	float Magnitude() const
	{
		return sqrt(SqMagnitude());
	}

	float SqMagnitude() const
	{
		return (_x * _x) + (_y * _y);
	}

	void Normalize()
	{
		//Check for this in order to avoid dividing by 0
		if (fabs(_x) < _EPSILON && fabs(_y) < _EPSILON)
		{
			return;
		}

		*this /= Magnitude();
	}

	Vec2f GetNormalize() const
	{
		return Vec2f(*this) /= Magnitude();
	}

private:
	float _x;
	float _y;

	static constexpr float _EPSILON = 1.0e-04f;
};

