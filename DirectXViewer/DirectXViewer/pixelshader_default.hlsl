#pragma pack_matrix( row_major )


Texture2D tex_diffuse : register(t0);
Texture2D tex_emissive : register(t1);
Texture2D tex_specular : register(t2);
SamplerState sampler_linear : register(s0);


cbuffer ConstantBuffer : register(b1)
{
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
	float4 world_pos : WORLDPOS;
	float4 eye_pos : EYEPOS;
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

    // get point light direction
    float3 light_dir = light_pos - input.world_pos.xyz;
    float sq_dist_light = dot(light_dir, light_dir);
    light_dir = light_dir / sqrt(sq_dist_light);

    // get eye direction
    float3 eye_dir = input.eye_pos.xyz - input.world_pos.xyz;
    float sq_dist_eye = dot(eye_dir, eye_dir);
    eye_dir = eye_dir / sqrt(sq_dist_eye);

    // calculate N dot L
    float3 norm = normalize(input.norm.xyz);
    float NdotL = dot(norm, light_dir);

    // calculate half-vector
    float3 half_vector = normalize(light_dir + eye_dir);

    // calculate N dot H
    float NdotH = dot(norm, half_vector);

    // calculate light intensitites
    float light_intensity = float4(light_color, 0.0f) * light_power / sq_dist_light;
    float diffuse_intensity = saturate(NdotL);
    float specular_intensity = pow(saturate(NdotH), 1 + surface_shininess);

    // sample material colors
    float4 mat_diffuse = tex_diffuse.Sample(sampler_linear, input.uv);
    float4 mat_emissive = tex_emissive.Sample(sampler_linear, input.uv);
    float4 mat_specular = tex_specular.Sample(sampler_linear, input.uv);

    // calculate light colors
    float4 ambient = mat_diffuse * ambient_light;
    float4 diffuse = mat_diffuse * diffuse_intensity * light_intensity;
    float4 emissive = mat_emissive;
    float4 specular = mat_specular * specular_intensity * light_intensity;

    diffuse.xyz -= specular.xyz;
    diffuse.xyz = saturate(diffuse.xyz);

    float4 color = ambient + diffuse + emissive + specular;

    output.color = color;
	return output;
}