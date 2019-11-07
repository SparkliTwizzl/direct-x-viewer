#pragma pack_matrix( row_major )


cbuffer ConstantBuffer : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 projection;
};


struct VSin
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};


struct VSout
{
	float4 pos : SV_POSITION;
	float4 norm : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 world_pos : WORLDPOS;
	float4 eye_pos : EYEPOS;
};


VSout main(VSin input)
{
	VSout output;

	output.world_pos = mul(float4(input.pos, 1.0f), world);

	output.pos = mul(output.world_pos, view);
	output.pos = mul(output.pos, projection);

	// dot products need to have order swapped
	// if they were written for column-major
	// since this shader uses row-major
	output.eye_pos.x = -dot(view[3].xyz, view[0].xyz);
	output.eye_pos.y = -dot(view[3].xyz, view[1].xyz);
	output.eye_pos.z = -dot(view[3].xyz, view[2].xyz);
	output.eye_pos.w = 1.0f;

	output.norm = mul(float4(input.norm, 0.0f), world);

	output.color = input.color;

	output.uv = input.uv;

	return output;
}