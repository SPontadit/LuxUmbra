#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform ViewProj
{
	mat4 viewProj;
} vp;

layout(push_constant) uniform Model
{
	mat4 model;
} m;

void main()
{
	gl_Position = vp.viewProj * m.model * vec4(inPosition, 1.0);
}