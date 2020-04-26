#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHT_MAX_COUNT 64

layout(location = 0) in vec3 inPositionWS;
layout(location = 1) in vec2 inTextureCoordinateLS;
layout(location = 2) in vec3 inNormalWS;
layout(location = 3) in vec3 inFragView;

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

layout(set = 1, binding = 0) uniform Material
{
	vec3 baseColor;
	float metallic;
	float perceptualRoughness;
	float reflectance;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedo;

//#define BASE_COLOR material.color.xyz
//#define METALLIC material.color.w
//#define PERCEPTUAL_ROUGHNESS material.parameter.x
//#define REFLECTANCE material.parameter.y
//layout(binding = 2) uniform Material
//{
//	vec4 color;
//	vec4 parameter;
//} material;



const float PI = 3.1415926;


vec3 RemapDiffuseColor(vec3 baseColor, float metallic);
vec3 GetF0(float reflectance, float metallic, vec3 baseColor);
float D_GGX(float VdotH, float roughness);
float V_SmithGGXCorrelated(float NdotV, float NdotL, float roughness);
vec3 F_Schlick(float NdotH, vec3 f0);


void main() 
{
	vec3 directColor  = vec3(0.0);

	for(int i = 0; i < pushConsts.lightCount; ++i)
	{
		vec3 lightDir = normalize(-lightBuffer.lights[i].parameter.xyz);
		vec3 viewDir = normalize(inFragView);
		vec3 h = normalize(viewDir + lightDir);
		vec3 normal = normalize(inNormalWS);
	
		float NdotH = max(dot(normal, h), 0.001);
		float NdotV = max(dot(normal, viewDir), 0.001);
		float NdotL = max(dot(normal, lightDir), 0.001);
		float LdotH = max(dot(lightDir, h), 0.001);
		float VdotH = max(dot(viewDir, h), 0.001);

		vec3 baseColor = pow(texture(albedo, inTextureCoordinateLS).rgb, vec3(2.2));
		float roughness = material.perceptualRoughness * material.perceptualRoughness;
		vec3 diffuseColor = RemapDiffuseColor(baseColor, material.metallic);
		vec3 F0 = GetF0(material.reflectance, material.metallic, baseColor);


		float D = D_GGX(NdotH, roughness);
		vec3 F = F_Schlick(LdotH, F0);
		float V = V_SmithGGXCorrelated(NdotV, NdotL, roughness);

		vec3 specular = (D * V) * F;
		vec3 Kdiff = vec3(1.0) - (F0 + (vec3(1.0) - F0) * pow(1.0 - NdotL, 5.0));

		directColor += (diffuseColor * Kdiff / PI + specular) * lightBuffer.lights[i].color * NdotL;
	}

	outColor = vec4(directColor, 1.0);

	outColor = pow(outColor, vec4(1.0/2.2));
}

float D_GGX(float NdotH, float roughness)
{
	float a = NdotH * roughness;
	float k = roughness / (1.0 - NdotH * NdotH + a * a);
	return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelated(float NdotV, float NdotL, float roughness)
{
	float a2 = roughness * roughness;
	float lambdaV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
	float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);

	return 0.5 / (lambdaV + lambdaL);
}

vec3 F_Schlick(float VdotH, vec3 f0)
{
	return f0 + (vec3(1.0) - f0) * pow(1.0 - VdotH, 5.0);
	float f = pow(1.0 - VdotH, 5.0);

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