#pragma once

#include "mathtypes.h"


namespace DirectXViewer
{
	namespace DebugRenderer
	{
		struct simple_vertex
		{
			float3 pos = { 0.0f, 0.0f, 0.0f };
			float4 color = { 1.0f, 1.0f, 1.0f, 0.0f };

			simple_vertex() = default;
			simple_vertex(const simple_vertex&) = default;

			inline simple_vertex(const float3& _p, const float4& _c) : pos{ _p }, color{ _c } {}
			inline simple_vertex(const float3& _p, const float3& _c) : pos{ _p }, color{ _c.x, _c.y, _c.z, 1.0f } {}
			inline simple_vertex(const float3& _p, uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255) : pos{ _p }, color{ _r / 255.0f, _g / 255.0f, _b / 255.0f, _a / 255.0f } {}
		};


		void add_debug_line(float3 _point_a, float3 _point_b, float4 _color_a, float4 _color_b);

		inline void add_debug_line(float3 _p, float3 _q, float4 _color) { add_debug_line(_p, _q, _color, _color); }

		void clear_debug_lines();

		const simple_vertex* get_debug_line_verts();

		size_t get_debug_line_vert_count();

		size_t get_debug_line_vert_capacity();
	}

}
