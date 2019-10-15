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

layout(binding = 1)uniform LightBuffer
{
	Light lights[LIGHT_MAX_COUNT];
} lightBuffer;

layout(push_constant) uniform PushConsts
{
	layout(offset = 64) int lightCount;
} pushConsts;

void main() 
{
	vec4 color  = vec4(0.0);

	for(int i = 0; i < pushConsts.lightCount; ++i)
	{
		color.xyz += lightBuffer.lights[i].color;
	}

	outColor = color;
}