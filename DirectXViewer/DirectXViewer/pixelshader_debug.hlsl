#pragma pack_matrix( row_major )


struct VSout
{
    float4 pos : SV_POSITION;
    float4 norm : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 world_pos : WORLD_POS;
    float4 cam_pos : CAM_POS;
};

#define PSin VSout


struct PSout
{
    float4 color : SV_TARGET;
};


static const float4 ambient_light = { 0.25f, 0.25f, 0.25f, 0.0f };

PSout main(PSin input)
{
    PSout output;

    output.color = input.color;
    return output;
}