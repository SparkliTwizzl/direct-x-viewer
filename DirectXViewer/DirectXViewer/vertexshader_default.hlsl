#pragma pack_matrix( row_major )

cbuffer ConstantBuffer : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 projection;
};

struct VSin
{

};

struct VSout
{

};

VSout main(VSin input)
{
	VSout output;

	return output;
}