#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinate;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out VsOut
{
	vec3 positionWS;
	vec2 textureCoordinateLS;
	vec3 normalWS;
	mat4 viewMatrix;
	mat3 textureToViewMatrix;
	vec4 shadowCoord;
} vsOut;


layout(binding = 0) uniform ViewProj
{
	mat4 view;
	mat4 proj;
	mat4 lightViewProj;
} vp;

layout(push_constant) uniform Model
{
	mat4 model;
} m;

void main() 
{
	vec4 fragPosition = m.model * vec4(inPosition, 1.0);
    gl_Position = vp.proj * vp.view * fragPosition;

	vsOut.positionWS = fragPosition.xyz;
	vsOut.viewMatrix =  vp.view;
	vsOut.textureCoordinateLS = inTextureCoordinate;

	mat3 normalMatrix = transpose(inverse(mat3(m.model)));
	vsOut.normalWS = normalMatrix * inNormal;

	mat3 modelToView = mat3(vp.view) * mat3(m.model);

	vec3 T = normalize(modelToView * inTangent);
	vec3 B = normalize(modelToView * inBitangent);
	vec3 N = normalize(modelToView * inNormal);

	vsOut.textureToViewMatrix = mat3(T, B, N);

	vsOut.shadowCoord = vp.lightViewProj * fragPosition;
}