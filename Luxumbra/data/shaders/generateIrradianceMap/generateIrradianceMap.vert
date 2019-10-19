#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinate;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPositionLS;

layout(push_constant) uniform PushConstants
{
	mat4 mvp;
	float deltaPhi;
	float deltaTheta;
} pushConstants;



void main() 
{
	outPositionLS = inPosition;

	gl_Position = pushConstants.mvp * vec4(inPosition, 1.0);
}