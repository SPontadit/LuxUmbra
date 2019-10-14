#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

layout(binding = 0) uniform ViewProj
{
	mat4 proj;
	mat4 view;
} vp;

void main() 
{
    gl_Position = vp.proj * vp.view * vec4(inPosition, 1.0);

	outNormal = inNormal;
}