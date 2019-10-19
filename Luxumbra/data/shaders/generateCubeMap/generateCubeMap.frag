#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPositionLS;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D envMap;

const float PI = 3.1415926;

vec2 RadialToTexCoords(vec3 position)
{
	float longitude = atan(position.z, position.x);
	float latitude = acos(position.y);

	const vec2 radianFactors = vec2(1.0 / (2.0 * PI), 1.0 / PI);

	return vec2(longitude, latitude) * radianFactors;
}

void main() 
{
	vec3 direction = normalize(inPositionLS);
	vec2 uv = RadialToTexCoords(direction);

	outColor = pow(texture(envMap, uv), vec4(1.0/2.2));
}