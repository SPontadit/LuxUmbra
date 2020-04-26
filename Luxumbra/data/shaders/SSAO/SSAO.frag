#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 textureCoordinate;
layout(location = 0) out float outColor;

layout(set = 0, binding = 0) uniform sampler2D positionMap;
layout(set = 0, binding = 1) uniform sampler2D normalMap;
layout(set = 0, binding = 2) uniform sampler2D SSAONoise;

layout(push_constant) uniform PushConstants
{
	mat4 proj;
	int kernelSize;
	float kernelRadius;
	float bias;
	float strenght;
};

#define MAX_SSAO_KERNEL_SIZE  32

layout(set = 0, binding = 3) uniform SSAOKernels
{
	vec4 samples[MAX_SSAO_KERNEL_SIZE];
};

float SSAO();

void main() 
{
	outColor = SSAO();
}

float SSAO()
{
	vec3 fragPosition = texture(positionMap, textureCoordinate).rgb;
	vec3 normal = normalize(texture(normalMap, textureCoordinate).rgb * 2.0 - 1.0);

	ivec2 textureDimension = textureSize(positionMap, 0);
	ivec2 noiseTextureDimension = textureSize(SSAONoise, 0);

	vec2 noiseUV = vec2(float(textureDimension.x) / float(noiseTextureDimension.x), float(textureDimension.y) / float(noiseTextureDimension.y)) * textureCoordinate;
	vec3 randomVec = texture(SSAONoise, noiseUV).xyz *  2.0 - 1.0;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(tangent, normal);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;

	for(int i = 0; i < kernelSize; ++i)
	{
		vec3 samplePosition = TBN * samples[i].xyz;
		samplePosition = fragPosition + samplePosition * kernelRadius;

		vec4 offset = vec4(samplePosition, 1.0);
		offset = proj * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = -texture(positionMap, offset.xy).w;
		float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(fragPosition.z - sampleDepth));

		occlusion += (sampleDepth >= samplePosition.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / float(kernelSize));

	return pow(occlusion, strenght);
}