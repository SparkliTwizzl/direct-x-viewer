#pragma pack_matrix( row_major )


Texture2D tex_diffuse : register(t0);
Texture2D tex_emissive : register(t1);
Texture2D tex_specular : register(t2);
Texture2D tex_normalmap : register(t3);

SamplerState sampler_linear : register(s0);


cbuffer ConstantBuffer : register(b1)
{
    float4 cam_pos;
    float3 light_pos;
    float3 light_color;
    float light_power;
    float surface_shininess;
};


struct VSout
{
    float4 pos : SV_POSITION;
    float4 norm : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float4 world_pos : WORLD_POS;
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

    // sample material colors
    float4 mat_diffuse = tex_diffuse.Sample(sampler_linear, input.uv);//+
    float4 mat_emissive = tex_emissive.Sample(sampler_linear, input.uv);//+
    float4 mat_specular = tex_specular.Sample(sampler_linear, input.uv);//+
    float4 mat_normalmap = tex_normalmap.Sample(sampler_linear, input.uv);//+


    // normalize pixel normal vector
    float3 norm = normalize(input.norm.xyz);//+


    // get vector from pixel to light (NOT NORMALIZED)
    float3 light_dir = light_pos - input.world_pos.xyz;//+

    // normalize light direction


    // get vector from pixel to camera (NOT NORMALIZED)
    float3 view_dir = cam_pos.xyz - input.world_pos.xyz;//+

    // normalize view direction


    // calculate half-angle vector
    float3 half_vec = normalize(light_dir + view_dir);//+


    // calculate N dot L
    float3 NdotL = dot(norm, light_dir);//+

    // calculate N dot H
    float3 NdotH = dot(norm, half_vec);//+


    // calculate light intensities
    float diffuse_intensity = saturate(NdotL);//+
    float specular_intensity = pow(saturate(NdotH), surface_shininess);


    // calculate light colors
    float4 ambient = ambient_light * mat_diffuse;//+
    float4 diffuse = mat_diffuse * diffuse_intensity;
    float4 emissive = mat_emissive;//+
    float4 specular = mat_specular * specular_intensity;
    float4 normalmap = mat_normalmap;


    float4 white = { 1, 1, 1, 0 };

    // get final color
    float4 color = ambient + emissive;
    color = white * specular_intensity;

    output.color = color;
    return output;
}