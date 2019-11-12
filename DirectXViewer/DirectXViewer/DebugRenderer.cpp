#include <array>

#include "DebugRenderer.h"


namespace
{
	constexpr size_t capacity = 4096;

	size_t count = 0;
	std::array<DirectXViewer::DebugRenderer::simple_vertex, capacity> line_verts;
}


namespace DirectXViewer
{
	namespace DebugRenderer
	{
		// adds line to array
		void add_debug_line(float3 _point_a, float3 _point_b, float4 _color_a, float4 _color_b)
		{
			line_verts[count++] = { _point_a, _color_a };
			line_verts[count++] = { _point_b, _color_b };
		}

		// resets vertex count
		void clear_debug_lines() { count = 0; }

		// returns address of first vertex
		const simple_vertex* get_debug_line_verts() { return &line_verts[0]; }

		// returns vertex count
		size_t get_debug_line_vert_count() { return count; }

		// returns vertex capacity
		size_t get_debug_line_vert_capacity() { return capacity; }
	}

}