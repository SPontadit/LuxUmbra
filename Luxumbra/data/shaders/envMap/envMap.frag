#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPositionLS;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPositionVS;
layout(location = 2) out vec4 outNormalVS;
layout(location = 3) out vec4 outIndirectColor;

layout(binding = 1) uniform samplerCube envMap;

void main() 
{
	outPositionVS = vec4(inPositionLS, 1.0);
	outColor = texture(envMap, inPositionLS);
}