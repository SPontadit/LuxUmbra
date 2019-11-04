#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outPositionLS;

layout(binding = 0) uniform ViewProj
{
	mat4 view;
	mat4 proj;
} vp;


void main() 
{
	outPositionLS = inPosition;

	gl_Position = (vp.proj * vp.view * vec4(inPosition, 0.0)).xyww;
}