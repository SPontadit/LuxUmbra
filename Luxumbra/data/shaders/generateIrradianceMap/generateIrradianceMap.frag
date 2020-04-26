#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPositionLS;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants
{
	mat4 mvp;
	float deltaPhi;
	float deltaTheta;
} pushConstants;

layout(binding = 0) uniform samplerCube envMap;

const float PI = 3.1415926535897932384626433832795;

void main() 
{
	vec3 N = normalize(inPositionLS);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;

	vec3 color = vec3(0.0);
	uint samplerCount = 0u;

	for(float phi = 0.0; phi < TWO_PI; phi += pushConstants.deltaPhi)
	{
		for(float theta = 0.0; theta < HALF_PI; theta += pushConstants.deltaTheta)
		{
			vec3 tmpVec = cos(phi) * right + sin(phi) * up;
			vec3 samplerVector = cos(theta) * N + sin(theta) * tmpVec;

			color += texture(envMap, samplerVector).rgb * cos(theta) * sin(theta);
			samplerCount++;
		}
	}

	outColor = vec4(PI * color / float(samplerCount), 1.0);
}