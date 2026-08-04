#pragma once
#include <cmath>

#define SMALL_NUMBER            (1.e-8f)
constexpr float PI = 3.1415926535f;
constexpr float PTM_RATIO = 50.0f;

struct Vector2
{
	float x = 0.0f;
	float y = 0.0f;

	Vector2() {}
	Vector2(float x, float y) : x(x), y(y) {}
	Vector2(POINT pt) : x((float)pt.x), y((float)pt.y) {}
	Vector2(const D2D1_POINT_2F& pt) :x(pt.x), y(pt.y) {}

	operator D2D1_POINT_2F() const
	{
		return D2D1::Point2F(x, y);
	}

	Vector2 operator+(const Vector2& other) const
	{
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator-(const Vector2& other) const
	{
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 operator*(float value) const
	{
		return Vector2(x * value, y * value);
	}

	void operator+=(const Vector2& other)
	{
		x += other.x;
		y += other.y;
	}

	void operator-=(const Vector2& other)
	{
		x -= other.x;
		y -= other.y;
	}

	void operator*=(float ratio)
	{
		x *= ratio;
		y *= ratio;
	}

	bool operator==(const Vector2& other) const
	{
		return (x == other.x && y == other.y);
	}

	float LengthSquared() const
	{
		return x * x + y * y;
	}

	float Length() const
	{
		return std::sqrt(LengthSquared());
	}

	void Normalize()
	{
		float length = Length();
		if (length < SMALL_NUMBER)
			return;

		x /= length;
		y /= length;
	}

	float Dot(const Vector2& other) const
	{
		return x * other.x + y * other.y;
	}

	float Cross(const Vector2& other) const
	{
		return x * other.y - y * other.x;
	}

	Vector2 Rotate(float radian) const
	{
		float cosA = std::cos(radian);
		float sinA = std::sin(radian);
		return Vector2(x * cosA - y * sinA, x * sinA + y * cosA);
	}
};

// 2D Rotation (Z-axis only)
struct Rotation
{
	float angle = 0.0f;
};

// 2D Scale
struct Scale
{
	float x = 1.0f;
	float y = 1.0f;
};
