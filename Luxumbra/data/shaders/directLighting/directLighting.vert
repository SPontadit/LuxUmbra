#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinate;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPositionWS;
layout(location = 1) out vec2 outTextureCoordinateLS;
layout(location = 2) out vec3 outNormalWS;
layout(location = 3) out vec3 outView;


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
	outPositionWS = fragPos;

    gl_Position = vp.proj * vp.view * vec4(fragPos, 1.0);

	mat3 normalMatrix = transpose(inverse(mat3(m.model)));
	outNormalWS = normalize(normalMatrix * inNormal);
	outView = -vec3(vp.view[3]) - fragPos;
	outTextureCoordinateLS = inTextureCoordinate;
}