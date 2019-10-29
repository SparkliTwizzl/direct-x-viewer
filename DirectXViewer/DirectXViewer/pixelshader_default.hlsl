cbuffer ConstantBuffer : register(b1)
{
	float4 light_pos;
	float3 light_color;
	float light_power;
	float surface_shininess;
	float3 pad;
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

struct PSout
{
    float4 color : SV_TARGET;
};

PSout main(VSout input)
{
	PSout output;

    output.color = float4(1.0f, 1.0f, 1.0f, 0.0f);

	return output;
}