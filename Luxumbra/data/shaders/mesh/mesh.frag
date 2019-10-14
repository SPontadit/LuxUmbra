#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inFragNormal;

layout(location = 0) out vec4 outColor;


void main() 
{
	vec4 color = vec4(normalize(inFragNormal), 1.0);

	outColor = color;
}