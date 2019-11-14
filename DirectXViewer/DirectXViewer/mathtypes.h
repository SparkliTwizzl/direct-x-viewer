#pragma once

struct float2
{
	float x;
	float y;

	inline static constexpr size_t size() { return 2; }

	inline float& operator[](int i) { return (&x)[i]; }
	inline float operator[](int i) const { return (&x)[i]; }

	inline bool operator==(float2 rhs) { return (x == rhs.x && y == rhs.y); }

	inline friend float2 operator+(float2 lhs, float2 rhs) { return { lhs.x + rhs.x, lhs.y + rhs.y }; }
	inline friend float2 operator-(float2 lhs, float2 rhs) { return { lhs.x - rhs.x, lhs.y - rhs.y }; }
	inline friend float2 operator*(float2 lhs, float2 rhs) { return { lhs.x * rhs.x, lhs.y * rhs.y }; }
	inline friend float2 operator/(float2 lhs, float2 rhs) { return { lhs.x / rhs.x, lhs.y / rhs.y }; }
	inline friend float2 operator*(float2 lhs, float rhs) { return { lhs.x * rhs, lhs.y * rhs }; }
	inline friend float2 operator/(float2 lhs, float rhs) { return { lhs.x / rhs, lhs.y / rhs }; }

	inline friend float dot(float2 lhs, float2 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }

	inline float2& operator+=(float2 rhs)
	{
		x += rhs.x;
		y += rhs.y;

		return *this;
	}
	inline float2& operator-=(float2 rhs)
	{
		x -= rhs.x;
		y -= rhs.y;

		return *this;
	}
	inline float2& operator*=(float2 rhs)
	{
		x *= rhs.x;
		y *= rhs.y;

		return *this;
	}
	inline float2& operator/=(float2 rhs)
	{
		x /= rhs.x;
		y /= rhs.y;

		return *this;
	}
	inline float2& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;

		return *this;
	}
	inline float2& operator/=(float rhs)
	{
		x /= rhs;
		y /= rhs;

		return *this;
	}
};

struct float3
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};

		float2 xy;
	};

	inline static constexpr size_t size() { return 3; }

	inline float& operator[](int i) { return (&x)[i]; }
	inline float operator[](int i) const { return (&x)[i]; }

	inline bool operator==(float3 rhs) { return (x == rhs.x && y == rhs.y && z == rhs.z); }

	inline friend float3 operator+(float3 lhs, float3 rhs) { return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z }; }
	inline friend float3 operator-(float3 lhs, float3 rhs) { return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z }; }
	inline friend float3 operator*(float3 lhs, float3 rhs) { return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z }; }
	inline friend float3 operator/(float3 lhs, float3 rhs) { return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z }; }
	inline friend float3 operator*(float3 lhs, float rhs) { return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs }; }
	inline friend float3 operator/(float3 lhs, float rhs) { return { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs }; }

	inline friend float dot(float3 lhs, float3 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }
	inline friend float3 cross(float3 lhs, float3 rhs) { return { lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x }; }

	inline float3& operator+=(float3 rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;

		return *this;
	}
	inline float3& operator-=(float3 rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;

		return *this;
	}
	inline float3& operator*=(float3 rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;

		return *this;
	}
	inline float3& operator/=(float3 rhs)
	{
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;

		return *this;
	}
	inline float3& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;

		return *this;
	}
	inline float3& operator/=(float rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;

		return *this;
	}
};

struct float4
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};

		float3 xyz;

		struct
		{
			float2 xy;
			float2 zw;
		};
	};

	inline float& operator[](int i) { return (&x)[i]; }
	inline float operator[](int i)const { return (&x)[i]; }

	inline static constexpr size_t size() { return 4; }


	inline bool operator==(float4 rhs) { return (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w); }

	inline friend float4 operator+(float4 lhs, float4 rhs) { return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w }; }
	inline friend float4 operator-(float4 lhs, float4 rhs) { return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w }; }
	inline friend float4 operator*(float4 lhs, float4 rhs) { return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w }; }
	inline friend float4 operator/(float4 lhs, float4 rhs) { return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w }; }
	inline friend float4 operator*(float4 lhs, float rhs) { return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs }; }
	inline friend float4 operator/(float4 lhs, float rhs) { return { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs }; }

	inline friend float dot(float4 lhs, float4 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w; }

	inline float4& operator+=(float4 rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;

		return *this;
	}
	inline float4& operator-=(float4 rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;

		return *this;
	}
	inline float4& operator*=(float4 rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		w *= rhs.w;

		return *this;
	}
	inline float4& operator/=(float4 rhs)
	{
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		w /= rhs.w;

		return *this;
	}
	inline float4& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;

		return *this;
	}
	inline float4& operator/=(float rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;

		return *this;
	}
};

struct alignas(8) float2_a : float2 {};

struct alignas(16) float3_a : float3 {};

struct alignas(16) float4_a : float4 {};

using float2x2 = std::array< float2, 2 >;
using float2x2_a = std::array< float2_a, 2 >;

using float2x3 = std::array< float2, 3 >;
using float2x3_a = std::array< float2_a, 3 >;

using float2x4 = std::array< float2, 4 >;
using float2x4_a = std::array< float2_a, 4 >;

using float3x2 = std::array< float3, 2 >;
using float3x2_a = std::array< float3_a, 2 >;

using float4x2 = std::array< float4, 2 >;
using float4x2_a = std::array< float4_a, 2 >;

using float3x3 = std::array< float3, 3 >;
using float3x3_a = std::array< float3_a, 3 >;

using float3x4 = std::array< float3, 4 >;
using float3x4_a = std::array< float3_a, 4 >;

using float4x3 = std::array< float4, 3 >;
using float4x3_a = std::array< float4_a, 3 >;

using float4x4 = std::array< float4, 4 >;
using float4x4_a = std::array< float4_a, 4 >;