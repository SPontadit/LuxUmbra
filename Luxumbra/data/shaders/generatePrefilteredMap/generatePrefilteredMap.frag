#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPositionLS;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants
{
	layout(offset = 72) float roughness;
	layout(offset = 76) int samplesCount;
} pushConstants;

layout(binding = 0) uniform samplerCube envMap;

const float PI = 3.1415926535897932384626433832795;

float Random(vec2 co);
vec3 PrefilterEnvMap(vec3 R, float roughness);
float RadicalInverse_VdC(uint bits);
vec2 Hammersley2D(uint i, uint N);
vec3 ImportanceSample_GGX(vec2 Xi, vec3 normal, float roughness);
float D_GGX(float NdotH, float roughness);


void main() 
{
	vec3 N = normalize(inPositionLS);
	outColor = vec4(PrefilterEnvMap(N, pushConstants.roughness), 1.0);
}

vec3 PrefilterEnvMap(vec3 R, float roughness)
{
	vec3 N = R;
	vec3 V = R;

	vec3 color = vec3(0.0);
	float totalWeight = 0.0;
	float envMapSize = float(textureSize(envMap, 0).s);

	for(uint i = 0u; i < pushConstants.samplesCount; ++i)
	{
		vec2 Xi = Hammersley2D(i, pushConstants.samplesCount);
		vec3 H = ImportanceSample_GGX(Xi, N, roughness);
		vec3 L = 2.0 *  dot(V, H) * H - V;
		float NdotL = clamp(dot(N, L), 0.0, 1.0);

		if (NdotL > 0.0)
		{
			float NdotH = clamp(dot(N, H), 0.0, 1.0);
			float VdotH = clamp(dot(V, H), 0.0, 1.0);

			float pdf = D_GGX(NdotH, roughness) * NdotH / (4.0 / VdotH) + 0.0001;

			float omegaS = 1.0 / (float(pushConstants.samplesCount) * pdf);
			float omegaP = 4.0 * PI / (6.0 * envMapSize * envMapSize);
			float mipLevel = roughness == 0.0 ? 0.0 : max(0.5 * log2(omegaS / omegaP) + 1.0, 0.0);

			color += textureLod(envMap, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	return (color / totalWeight);
}

float Random(vec2 co)
{
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt= dot(co.xy ,vec2(a,b));
	float sn= mod(dt,3.14);
	return fract(sin(sn) * c);
}

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}


vec2 Hammersley2D(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSample_GGX(vec2 Xi, vec3 normal, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x + Random(normal.xz) * 0.1;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, normal));
	vec3 bitangent = normalize(cross(normal, tangent));

	return normalize(tangent * H.x + bitangent * H.y + normal * H.z);
}

float D_GGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	
	float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;

	return a2 / (PI * denom * denom);
}