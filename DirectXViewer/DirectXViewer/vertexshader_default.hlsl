#pragma pack_matrix( row_major )


cbuffer ConstantBuffer : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 projection;
    float4x4 worldIT;
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
	float4 world_pos : WORLD_POS;
};


VSout main(VSin input)
{
	VSout output;

	output.world_pos = mul(float4(input.pos, 1.0f), world);

	output.pos = mul(output.world_pos, view);
	output.pos = mul(output.pos, projection);

	output.norm = mul(float4(input.norm, 0.0f), world);

	output.color = input.color;

	output.uv = input.uv;

	return output;
}