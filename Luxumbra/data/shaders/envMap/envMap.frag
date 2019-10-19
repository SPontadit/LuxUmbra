#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPositionLS;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform samplerCube envMap;

void main() 
{
	outColor = pow(texture(envMap, inPositionLS), vec4(1.0));
}