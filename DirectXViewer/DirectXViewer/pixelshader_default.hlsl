#pragma pack_matrix( row_major )


Texture2D tex_diffuse : register(t0);
Texture2D tex_emissive : register(t1);
Texture2D tex_specular : register(t2);
Texture2D tex_normalmap : register(t3);

SamplerState sampler_linear : register(s0);


cbuffer ConstantBuffer : register(b1)
{
    float3 light_pos;
    float light_power;
    float3 light_color;
    float surface_shininess;
};


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

    float4 mat_diffuse = tex_diffuse.Sample(sampler_linear, input.uv);
    float4 mat_emissive = tex_emissive.Sample(sampler_linear, input.uv);
    float4 mat_specular = tex_specular.Sample(sampler_linear, input.uv);
    float4 mat_normalmap = tex_normalmap.Sample(sampler_linear, input.uv);

    //float3 norm = normalize(input.norm.xyz); // from vertex
	//float3 norm = normalize(float3(2.0f * (mat_normalmap.x - 0.5f), 2.0f * (mat_normalmap.y - 0.5f), 2.0f * (mat_normalmap.z - 0.5f))); // from normal map
	float3 norm = normalize(input.norm.xyz + float3(2.0f * (mat_normalmap.x - 0.5f), 2.0f * (mat_normalmap.y - 0.5f), 2.0f * (mat_normalmap.z - 0.5f))); // vertex + normalmap

    float3 light_dir = light_pos - input.world_pos.xyz; // vector from pixel to light (not normalized)
	float sq_dist_light = dot(light_dir, light_dir); // squared distance from pixel to light (used for attenuation)
	light_dir = normalize(light_dir);


    // get vector from pixel to camera
    float3 view_dir = normalize(input.cam_pos.xyz - input.world_pos.xyz);


    float3 half_angle_vec = normalize(light_dir + view_dir);
    float NdotL = dot(norm, light_dir);
    float NdotH = dot(norm, half_angle_vec);

	float intensity_light = light_power / sq_dist_light;
	float intensity_diffuse = saturate(NdotL);
    float intensity_specular = pow(saturate(NdotH), surface_shininess);

	float4 color_light = float4(light_color, 0.0f) * intensity_light;
    float4 color_ambient = ambient_light * mat_diffuse;
    float4 color_diffuse = (mat_diffuse * intensity_diffuse) * color_light;
    float4 color_emissive = mat_emissive;
    float4 color_specular = (mat_specular * intensity_specular) * color_light;

	color_diffuse.xyz = saturate(color_diffuse.xyz - color_specular.xyz);

	float4 color_final = color_ambient + color_diffuse + color_emissive + color_specular;

    output.color = color_final;
    return output;
}