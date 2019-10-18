#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 outPositionWS;

layout(binding = 0) uniform ViewProj
{
	mat4 proj;
	mat4 view;
} vp;

vec2 positions[4] = vec2[](
    vec2(-1.0, 1.0),
	vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
	vec2(1.0, -1.0)
);


void main() 
{
    gl_Position = vec4(positions[gl_VertexIndex], 1.0, 1.0);


	mat4 NDCToWorld = inverse(vp.proj * mat4(mat3(vp.view)));

	outPositionWS = (NDCToWorld * gl_Position).xyz;
}