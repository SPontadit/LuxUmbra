#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform ViewProj
{
	mat4 viewProj[6];
} vp;

layout(push_constant) uniform PushConsts
{
	layout(offset = 0)  mat4 model;
	layout(offset = 64) uint vpIndex;
} pushConsts;

void main()
{
	gl_Position = vp.viewProj[pushConsts.vpIndex] * pushConsts.model * vec4(inPosition, 1.0);
}