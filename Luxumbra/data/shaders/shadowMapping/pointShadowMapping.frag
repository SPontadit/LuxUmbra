#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPositionLS;

layout(location = 0) out float linearDepth;

void main()
{
	linearDepth = length(inPositionLS.xyz);
}