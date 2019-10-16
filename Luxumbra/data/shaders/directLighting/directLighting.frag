#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHT_MAX_COUNT 64

layout(location = 0) in vec3 inFragNormal;

layout(location = 0) out vec4 outColor;

struct Light
{
	vec4 parameter;
	vec3 color;
};

layout(binding = 1) uniform LightBuffer
{
	Light lights[LIGHT_MAX_COUNT];
} lightBuffer;

layout(push_constant) uniform PushConsts
{
	layout(offset = 64) int lightCount;
} pushConsts;

layout(binding = 2) uniform Material
{
	vec3 baseColor;
	int metallic;
	float perceptualRoughness;
	float reflectance;
} material;


const float PI = 3.1415926;


vec3 RemapDiffuseColor();
vec3 GetF0();
float N_GGX(float NdotH, float roughness);
float V_SmithGGXCorrelated(float NdotV, float NdotL, float roughness);
vec3 F_Schlick(float NdotH, vec3 f0);

void main() 
{
	vec4 color  = vec4(0.0);

	for(int i = 0; i < pushConsts.lightCount; ++i)
	{
		color.xyz += lightBuffer.lights[i].color;
	}

	outColor = color;
}

float N_GGX(float NdotH, float roughness)
{
	float a = NdotH * roughness;
	float k = roughness / (1.0 - NdotH * NdotH + a * a);
	return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelated(float NdotV, float NdotL, float roughness)
{
	float a2 = roughness * roughness;
	float lambdaV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
	float lambdaL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);

	return 0.5 / (lambdaV + lambdaL);
}

vec3 F_Schlick(float NdotH, vec3 f0)
{
	float f = pow(1.0 - NdotH, 5.0);

	return f + f0 * (1.0 - f);
}

vec3 RemapDiffuseColor(vec3 baseColor, float metallic)
{
	return (1.0 - metallic) * baseColor;
}

vec3 GetF0(float reflectance, float metallic, vec3 baseColor)
{
	return (0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor * metallic);
}