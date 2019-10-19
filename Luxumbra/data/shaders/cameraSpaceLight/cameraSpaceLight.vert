#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinate;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out VsOut
{
	vec2 textureCoordinateLS;
	mat4 viewMatrix;
	mat3 TBN;
} vsOut;


layout(binding = 0) uniform ViewProj
{
	mat4 proj;
	mat4 view;
} vp;

layout(push_constant) uniform Model
{
	mat4 model;
} m;

void main() 
{
    gl_Position = vp.proj * vp.view * m.model * vec4(inPosition, 1.0);

	vsOut.viewMatrix =  vp.view;
	vsOut.textureCoordinateLS = inTextureCoordinate;

	vec3 T = normalize(mat3(m.model) * inTangent);
	vec3 B = normalize(mat3(m.model) * inBitangent);
	vec3 N = normalize(mat3(m.model) * inNormal);

	mat3 TBN = transpose(mat3(T, B, N));

	vsOut.TBN = TBN;
}