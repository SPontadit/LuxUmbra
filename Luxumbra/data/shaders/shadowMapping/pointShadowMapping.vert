#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 outPositionLS;

layout(set = 0, binding = 0) uniform LightInfo
{
	mat4 view[6];
	mat4 proj;
} lightInfo;

layout(push_constant) uniform PushConsts
{
	layout(offset = 0)  mat4 model;
	layout(offset = 64) uint vpIndex;
} pushConsts;

void main()
{
	outPositionLS = lightInfo.view[pushConsts.vpIndex] * pushConsts.model * vec4(inPosition, 1.0);
	gl_Position = lightInfo.proj * outPositionLS;
}