#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHT_MAX_COUNT 64

layout(location = 0) in FsIn
{
	vec3 positionWS;
	vec2 textureCoordinateLS;
	vec3 normalWS;
	mat4 viewMatrix;
	mat3 textureToViewMatrix;
	vec4 shadowCoord;
} fsIn;

layout(location = 0) out vec4 outColor;

struct Light
{
	vec4 parameter;
	vec3 color;
};

layout(set = 0, binding = 1) uniform LightBuffer
{
	Light lights[LIGHT_MAX_COUNT];
};

layout(set = 0, binding = 2) uniform samplerCube irradianceMap;
layout(set = 0, binding = 3) uniform samplerCube prefilteredMap;
layout(set = 0, binding = 4) uniform sampler2D BRDFLut;
layout(set = 0, binding = 5) uniform sampler2D shadowMap;

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
	int useTextureMask;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedo;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessMap;
layout(set = 1, binding = 4) uniform sampler2D ambientOcclusionMap;

//#define BASE_COLOR material.color.xyz
//#define METALLIC material.color.w
//#define PERCEPTUAL_ROUGHNESS material.parameter.x
//#define REFLECTANCE material.parameter.y
//layout(binding = 2) uniform Material
//{
//	vec4 color;
//	vec4 parameter;
//} material;

#define ROUGHNESS_MASK = 0x01
#define METALLIC_MASK = 0x02

const float PI = 3.1415926;

float Shadow(vec4 shadowCoord);
vec4 CameraSpace();

// PBR
vec3 RemapDiffuseColor(vec3 baseColor, float metallic);
vec3 GetF0(float reflectance, float metallic, vec3 baseColor);
float D_GGX(float VdotH, float roughness);
float V_SmithGGXCorrelated(float NdotV, float NdotL, float roughness);
vec3 F_Schlick(float NdotH, vec3 f0);
float F_Schlick(float NdotH, float f0, float f90);
vec3 F_SchlickRoughness(float VdotH, vec3 f0, float roughness);
float Fd_Lambert();
float Fd_Burley(float NdotV, float NdotL, float LdotH, float roughness);
vec3 PrefilteredReflection(vec3 R, float roughness);


void main() 
{
	outColor = CameraSpace();

	// Tonemapping
	//outColor = outColor / (outColor + vec4(1.0));

	//outColor = pow(outColor, vec4(1.0/2.2));
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, fsIn.textureCoordinateLS).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fsIn.positionWS);
    vec3 Q2  = dFdy(fsIn.positionWS);
    vec2 st1 = dFdx(fsIn.textureCoordinateLS);
    vec2 st2 = dFdy(fsIn.textureCoordinateLS);

    vec3 N   = normalize(fsIn.normalWS);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN  * tangentNormal);
}

float Shadow(vec4 shadowCoord)
{
	if (abs(shadowCoord.x) > 1.0 || abs(shadowCoord.y) > 1.0 || abs(shadowCoord.z) > 1.0)
		return 0.0;

	vec2 shadowUV = shadowCoord.xy * 0.5 + 0.5;
	if (shadowCoord.z > texture(shadowMap, shadowUV).x)
		return 0.0;

	return 1.0;
}

vec4 CameraSpace()
{
	vec3 directColor  = vec3(0.0);
	vec4 textureColor = texture(albedo, fsIn.textureCoordinateLS);

	float inv = gl_FrontFacing ? 1.0 : -1.0;

//	vec3 viewWS = -fsIn.viewMatrix[3].xyz;
//	vec3 viewDir = normalize(-fsIn.positionWS);
//	viewDir = mat3(fsIn.viewMatrix) * viewDir;
//	viewDir = normalize(viewDir);
	vec3 viewDir = vec3(0.0, 0.0, 1.0);

	vec3 normal = texture(normalMap, fsIn.textureCoordinateLS).rgb;
	normal = normalize(normal * 2.0 - 1.0) * inv;
	normal = normalize(fsIn.textureToViewMatrix * normal);

	float NdotV = max(dot(normal, viewDir), 0.001);
	
	vec3 baseColor = pow(textureColor.rgb * material.baseColor, vec3(2.2));
	baseColor *= textureColor.a;

//	float roughness = material.perceptualRoughness * material.perceptualRoughness;

	// Perceptual dans la texture ?

	float roughness = (material.useTextureMask & ROUGHNESS_MASK) == ROUGHNESS_MASK ? texture(metallicRoughnessMap, fsIn.textureCoordinateLS).g : material.perceptualRoughness * material.perceptualRoughness;
	float metallic = (material.useTextureMask & METALLIC_MASK) == METALLIC_MASK ? texture(metallicRoughnessMap, fsIn.textureCoordinateLS).b : material.metallic;
	vec3 diffuseColor = RemapDiffuseColor(baseColor, metallic);
	vec3 F0 = GetF0(material.reflectance, metallic, baseColor);

	for(int i = 0; i < pushConsts.lightCount; ++i)
	{
		vec3 lightDir;
		vec3 radiance;

		// Directional
		if(lights[i].parameter.w == 0.0)
		{
			lightDir = normalize(mat3(fsIn.viewMatrix) * -lights[i].parameter.xyz);
			radiance = lights[i].color;

		}
		else
		{
			vec3 lightPosVS = mat3(fsIn.viewMatrix) * (lights[i].parameter.xyz - fsIn.positionWS);
			lightDir = normalize(lightPosVS);
			float distance = length(lightPosVS);
			float attenuation = 1.0 / (distance * distance);
			
			radiance = lights[i].color * attenuation;
		}

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

		//vec3 directDiffuseColor = diffuseColor * Kdiff * Fd_Burley(NdotV, NdotL, LdotH, roughness);
		vec3 directDiffuseColor = diffuseColor * Kdiff * Fd_Lambert();

		directColor += (directDiffuseColor + specular) * radiance * NdotL;
	}

	float shadow = Shadow(fsIn.shadowCoord / fsIn.shadowCoord.w);


	viewDir = -fsIn.viewMatrix[3].xyz  * mat3(fsIn.viewMatrix);
	viewDir = normalize(viewDir - fsIn.positionWS);

	normal = transpose(mat3(fsIn.viewMatrix)) * normal;
	normal = normalize(normal);

	vec3 R = reflect(-viewDir, normal);

	NdotV = max(dot(normal, viewDir), 0.001);

	vec2 BRDF = texture(BRDFLut, vec2(NdotV, roughness)).rg;
	vec3 reflection = PrefilteredReflection(R, roughness).rgb;
	vec3 irradiance = texture(irradianceMap, normal).xyz;
	
	vec3 F = F_SchlickRoughness(NdotV, F0, roughness);
	//vec3 indirectSpecularColor = reflection * (F * BRDF.x + BRDF.y);
	vec3 indirectSpecularColor = reflection * mix(BRDF.xxx, BRDF.yyy, F0);

	vec3 Kdiff = 1.0 - F;
	vec3 indirectDiffuseColor = irradiance * diffuseColor * Kdiff;
	vec3 indirectColor = indirectDiffuseColor + indirectSpecularColor;
	
	float ao = texture(ambientOcclusionMap, fsIn.textureCoordinateLS).r;

	indirectColor *= ao;

	return vec4(directColor * shadow + indirectColor, textureColor.a);
}

vec3 PrefilteredReflection(vec3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 10.0;
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);

	vec3 a = textureLod(prefilteredMap, R, lodf).rgb;
	vec3 b = textureLod(prefilteredMap, R, lodc).rgb;

	return mix(a, b, lod - lodf);
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
}

vec3 F_SchlickRoughness(float VdotH, vec3 f0, float roughness)
{
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - VdotH, 5.0);
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