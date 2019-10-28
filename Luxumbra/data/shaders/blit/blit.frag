#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 textureCoordinate;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D renderTarget;

layout(push_constant) uniform PushConstants
{
	float exposure;
};

vec3 ACESFilm(vec3 x);
vec3 Uncharted2Tonemap(vec3 x);
vec3 Reinhard(vec3 x);

void main() 
{
	vec3 color = texture(renderTarget, textureCoordinate).rgb;
	color = ACESFilm(0.6 * color * exposure);
	//color = Reinhard(color);
	
//	outColor = pow(outColor, vec4(2.2));

//	color = Uncharted2Tonemap(color * exposure);
//	color = color * (1.0 / Uncharted2Tonemap(vec3(11.2)));
	outColor = pow(vec4(color, 1.0), vec4(1.0/2.2));
	return;

}

vec3 Reinhard(vec3 x)
{
	return x / (x + vec3(1.0));
}

vec3 ACESFilm(vec3 x)
{
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;

	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 Uncharted2Tonemap(vec3 x)
{
	float a = 0.15;
	float b = 0.50;
	float c = 0.10;
	float d = 0.20;
	float e = 0.02;
	float f = 0.30;

	return ((x*(a*x+c*b)+d*e)/(x*(a*x+b)+d*f))-e/f;
}
