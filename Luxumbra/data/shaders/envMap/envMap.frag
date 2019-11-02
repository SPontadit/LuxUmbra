#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPositionLS;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPositionWS;
layout(location = 2) out vec4 outNormalWS;

layout(binding = 1) uniform samplerCube envMap;

void main() 
{
	outColor = texture(envMap, inPositionLS);
}