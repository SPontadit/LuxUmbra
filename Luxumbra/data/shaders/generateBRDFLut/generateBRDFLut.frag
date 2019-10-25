#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;


layout(push_constant) uniform PushConstants
{
	int samplesCount;
} pushConstants;

const float PI = 3.1415926535897932384626433832795;


float Random(vec2 co);
float RadicalInverse_VdC(uint bits);
vec2 Hammersley2D(uint i, uint N);
vec3 ImportanceSample_GGX(vec2 Xi, vec3 normal, float roughness);
float G_SchlickSmithGGX(float NdotL, float NdotV, float roughness);
vec2 BRDF(float NdotV, float roughness);


void main() 
{
	outColor = vec4(BRDF(inTexCoord.s, 1.0 - inTexCoord.t), 0.0, 1.0);
}

vec2 BRDF(float NdotV, float roughness)
{
	const vec3 normal = vec3(0.0, 0.0, 1.0);
	vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

	vec2 LUT = vec2(0.0);

	for(uint i = 0u; i < pushConstants.samplesCount; ++i)
	{
		vec2 Xi = Hammersley2D(i, pushConstants.samplesCount);
		vec3 H = ImportanceSample_GGX(Xi, normal, roughness);
		vec3 L = 2.0 * dot(V, H) * H - V;

		float NdotL = max(dot(normal, L), 0.001);
		float NdotV = max(dot(normal, V), 0.001);
		float VdotH = max(dot(V, H), 0.001);
		float NdotH = max(dot(normal, H), 0.001);
	
		if (NdotL > 0.0)
		{
			float G = G_SchlickSmithGGX(NdotV, NdotL, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0 - VdotH, 5.0);

			LUT += vec2(Fc * G_Vis, G_Vis);
			//LUT += vec2((1.0 - Fc) * G_Vis, Fc * G_Vis);

		}
	}

	return LUT / float(pushConstants.samplesCount);
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

float G_SchlickSmithGGX(float NdotL, float NdotV, float roughness)
{
	float k = (roughness * roughness) / 2.0;
	
	float GL = NdotL / (NdotL * (1.0 - k) + k);
	float GV = NdotV / (NdotV * (1.0 - k) + k);

	return GL * GV;
}