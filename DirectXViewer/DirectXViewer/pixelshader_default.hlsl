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

};

struct PSout
{

};

PSout main(VSout input)
{
	PSout output;

	return output;
}