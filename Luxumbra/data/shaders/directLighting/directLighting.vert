#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outView;
layout(location = 2) out vec3 outWorldPosition;


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
	vec3 fragPos = (m.model * vec4(inPosition, 1.0)).xyz;
	outWorldPosition = fragPos;

	mat3 normalMatrix = transpose(inverse(mat3(m.model)));
	outNormal = normalMatrix * inNormal;
	outView = -vec3(vp.view[3]) - fragPos;
}