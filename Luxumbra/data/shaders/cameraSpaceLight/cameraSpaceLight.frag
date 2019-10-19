#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHT_MAX_COUNT 64

layout(location = 0) in FsIn
{
	vec2 textureCoordinateLS;
	mat4 viewMatrix;
	mat3 TBN;
} fsIn;

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
layout(set = 1, binding = 2) uniform sampler2D normalMap;

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

vec3 WorldSpace();
vec3 CameraSpace();
vec3 RemapDiffuseColor(vec3 baseColor, float metallic);
vec3 GetF0(float reflectance, float metallic, vec3 baseColor);
float D_GGX(float VdotH, float roughness);
float V_SmithGGXCorrelated(float NdotV, float NdotL, float roughness);
vec3 F_Schlick(float NdotH, vec3 f0);
float F_Schlick(float NdotH, float f0, float f90);
float Fd_Lambert();
float Fd_Burley(float NdotV, float NdotL, float LdotH, float roughness);


void main() 
{
	outColor = vec4(CameraSpace(), 1.0);
	outColor = pow(outColor, vec4(1.0/2.2));
}

vec3 CameraSpace()
{
	vec3 directColor  = vec3(0.0);
	
	vec3 normal = texture(normalMap, fsIn.textureCoordinateLS).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 viewDir = normalize(fsIn.TBN * (-fsIn.viewMatrix[3].xyz));
	float NdotV = max(dot(normal, viewDir), 0.001);

	vec3 baseColor = pow(texture(albedo, fsIn.textureCoordinateLS).rgb * material.baseColor, vec3(2.2));
	float roughness = material.perceptualRoughness * material.perceptualRoughness;
	vec3 diffuseColor = RemapDiffuseColor(baseColor, material.metallic);
	vec3 F0 = GetF0(material.reflectance, material.metallic, baseColor);

	for(int i = 0; i < pushConsts.lightCount; ++i)
	{
		vec3 lightDir = normalize(fsIn.TBN * mat3(fsIn.viewMatrix) * -lightBuffer.lights[i].parameter.xyz);
		vec3 h = normalize(viewDir + lightDir);
	
		float NdotH = max(dot(normal, h), 0.001);
		float NdotL = max(dot(normal, lightDir), 0.001);
		float LdotH = max(dot(lightDir, h), 0.001);
		float VdotH = max(dot(viewDir, h), 0.001);

		float D = D_GGX(NdotH, roughness);
		vec3 F = F_Schlick(LdotH, F0);
		float V = V_SmithGGXCorrelated(NdotV, NdotL, roughness);

		vec3 specular = (D * V) * F;
		vec3 Kdiff = vec3(1.0) - (F0 + (vec3(1.0) - F0) * pow(1.0 - NdotL, 5.0));

		directColor += (diffuseColor * Kdiff * Fd_Burley(NdotV, NdotL, LdotH, roughness) + specular) * lightBuffer.lights[i].color * NdotL;
	}

	return directColor;
}

float Fd_Lambert()
{
	return 1.0 / PI;
}

float Fd_Burley(float NdotV, float NdotL, float LdotH, float roughness)
{
	float f90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
	float lightScatter = F_Schlick(NdotL, 1.0, f90);
	float viewScatter = F_Schlick(NdotV, 1.0, f90);
	return lightScatter * viewScatter * (1.0 / PI);
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

float F_Schlick(float VdotH, float f0, float f90)
{
	return f0 + (f90 - f0) * pow(1.0 - VdotH, 5.0);
}

vec3 RemapDiffuseColor(vec3 baseColor, float metallic)
{
	return (1.0 - metallic) * baseColor;
}

vec3 GetF0(float reflectance, float metallic, vec3 baseColor)
{
	return (0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor * metallic);
}