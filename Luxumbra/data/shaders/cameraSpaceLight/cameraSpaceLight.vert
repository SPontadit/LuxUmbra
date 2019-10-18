#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinate;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPositionVS;
layout(location = 1) out vec2 outTextureCoordinateLS;
layout(location = 2) out vec3 outNormalVS;
layout(location = 3) out mat4 outViewMatrice;


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
	vec3 fragPos = (m.model * vec4(inPosition, 1.0)).xyz;
	outPositionVS = mat3(vp.view) * fragPos;

    gl_Position = vp.proj * vp.view * vec4(fragPos, 1.0);

	outViewMatrice =  vp.view;

	mat3 normalMatrix = transpose(inverse(mat3(vp.view * m.model)));
	outNormalVS = normalize(normalMatrix * inNormal);
	
	outTextureCoordinateLS = inTextureCoordinate;
}