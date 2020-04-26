#version 450
#extension GL_ARB_separate_shader_objects : enable


//layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec2 inTexCoord;
//layout(location = 2) in vec2 inNormal;

vec2 vertex[3] = vec2[]
(

	vec2(0.0, -0.5),
	vec2(-0.5, 0.5),
	vec2(0.5, 0.5)
);


void main() 
{
    gl_Position = vec4(vertex[gl_VertexIndex], 0.0, 1.0);
}