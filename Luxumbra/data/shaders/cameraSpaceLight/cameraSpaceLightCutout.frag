#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in FsIn
{
	vec3 positionWS;
	vec3 positionVS;
	vec2 textureCoordinateLS;
	vec3 normalWS;
	mat4 viewMatrix;
	mat3 textureToViewMatrix;
	float nearPlane;
	float farPlane;
} fsIn;

layout(set = 1, binding = 1) uniform sampler2D albedo;

void main() 
{
	if(texture(albedo, fsIn.textureCoordinateLS).a < 1.0)
		discard;
}