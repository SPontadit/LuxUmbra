#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 textureCoordinate;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D renderTarget;
layout(set = 0, binding = 1) uniform sampler2D SSAOMap;
layout(set = 0, binding = 2) uniform sampler2D IndirectColorMap;

#define SSAO_KERNEL_SIZE  32
#define SSAO_RADIUS 0.5

layout(push_constant) uniform PushConstants
{
	vec2 inverseScreenSize;
	int toneMapping;
	float exposure;
	float splitViewRatio;
	int splitViewMask;
	float FXAAContrastThreshold;
	float FXAARelativeThreshold;
};



#define SPLIT_VIEW_TONE_MAPPING_MASK 1
#define SPLIT_VIEW_FXAA_MASK 2
#define SPLIT_VIEW_SSAO_MASK 4
#define SPLIT_VIEW_SSAO_ONLY_MASK 8

#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.75

vec3 ACESFilm(vec3 x);
vec3 Uncharted2Tonemap(vec3 x);
vec3 Reinhard(vec3 x);
vec3 ToneMapGammaCorrect(vec3 color);
float BlurSSAO();
vec3 FXAA();
float RGBToLuma(vec3 rgb);
float Quality(int i);

void main() 
{
	outColor =  vec4(FXAA(), 1.0);

	if (splitViewMask != 0)
	{
		if ((splitViewMask & SPLIT_VIEW_SSAO_ONLY_MASK) == SPLIT_VIEW_SSAO_ONLY_MASK)
		{
			outColor = vec4(vec3(BlurSSAO()), 1.0);
			return;
		}

		if (textureCoordinate.x < splitViewRatio)
		{
			if ((splitViewMask & SPLIT_VIEW_FXAA_MASK) == SPLIT_VIEW_FXAA_MASK)
				outColor = vec4(FXAA(), 1.0);
			else
				outColor = vec4(ToneMapGammaCorrect(texture(renderTarget, textureCoordinate).rgb), 1.0);
		}
		else
		{
			vec4 indirect = texture(IndirectColorMap, textureCoordinate);
			outColor = pow(texture(renderTarget, textureCoordinate) + indirect, vec4(1.0/2.2));
		}

		if (textureCoordinate.x < splitViewRatio + 0.001 && textureCoordinate.x > splitViewRatio - 0.001)
			outColor = vec4(1.0);
	}
}

float BlurSSAO2()
{
	vec2 texelSize = vec2(1.0) / textureSize(SSAOMap, 0);
	vec2 f = fract(textureCoordinate * textureSize(SSAOMap, 0));
	vec2 uv = textureCoordinate + (0.5 - f) * texelSize;

	float tl = texture(SSAOMap, uv).r;
	float tr = texture(SSAOMap, uv + vec2(texelSize.x, 0.0)).r;
	float bl = texture(SSAOMap, uv + vec2(0.0, texelSize.y)).r;
	float br = texture(SSAOMap, uv + vec2(texelSize.x, texelSize.y)).r;

	float tA = mix(tl, tr, f.x);
	float tB = mix(bl, br, f.x);

	return mix(tA, tB, f.y);
}

float BlurSSAO()
{
	int blurRange = 2;
	vec2 texelSize = 1.0 / vec2(textureSize(SSAOMap, 0));
	float result = 0.0;
	for (int x = -blurRange; x < blurRange; x++) 
	{
		for (int y = -blurRange; y < blurRange; y++) 
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(SSAOMap, textureCoordinate + offset).r;
		}
	}

	return (result / float(blurRange * 8.0));
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

vec3 ToneMapGammaCorrect(vec3 color)
{
	float ssao = BlurSSAO();

	if(splitViewMask != 0)
		ssao = ((splitViewMask & SPLIT_VIEW_SSAO_MASK) == SPLIT_VIEW_SSAO_MASK) ? ssao : 1.0;

	vec3 indirect = texture(IndirectColorMap, textureCoordinate).rgb * ssao;

	if (splitViewMask == 0 || (splitViewMask & SPLIT_VIEW_TONE_MAPPING_MASK) == SPLIT_VIEW_TONE_MAPPING_MASK)
	{
		switch(toneMapping)
		{
			case 0: 
			{
				return pow(ACESFilm(0.6 * (indirect + color) * exposure), vec3(1.0/2.2));
			}
			case 1: 
			{
				return pow(Uncharted2Tonemap((indirect + color) * exposure), vec3(1.0/2.2));
			}
			case 2:
			{
				return pow(Reinhard((indirect + color) * exposure), vec3(1.0/2.2));
			}
		}
	}
	else
		return pow(indirect + color, vec3(1.0/2.2));
}

vec3 FXAA()
{
	vec3 colorCenter = ToneMapGammaCorrect(texture(renderTarget, textureCoordinate).rgb);

	float lumaCenter = RGBToLuma(colorCenter);

	float lumaDown = RGBToLuma(ToneMapGammaCorrect(textureOffset(renderTarget, textureCoordinate, ivec2(0, -1)).rgb));
	float lumaUp = RGBToLuma(ToneMapGammaCorrect(textureOffset(renderTarget, textureCoordinate, ivec2(0, 1)).rgb));
	float lumaLeft = RGBToLuma(ToneMapGammaCorrect(textureOffset(renderTarget, textureCoordinate, ivec2(-1, 0)).rgb));
	float lumaRight = RGBToLuma(ToneMapGammaCorrect(textureOffset(renderTarget, textureCoordinate, ivec2(1, 0)).rgb));

	float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
	float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));

	float deltaLuma = lumaMax - lumaMin;

	if (deltaLuma < max(FXAAContrastThreshold, lumaMax * FXAARelativeThreshold))
		return colorCenter;

	float lumaDownLeft = RGBToLuma(ToneMapGammaCorrect(textureOffset(renderTarget, textureCoordinate, ivec2(-1, -1)).rgb));
	float lumaUpRight = RGBToLuma(ToneMapGammaCorrect(textureOffset(renderTarget, textureCoordinate, ivec2(1, 1)).rgb));
	float lumaDownRight = RGBToLuma(ToneMapGammaCorrect(textureOffset(renderTarget, textureCoordinate, ivec2(1, -1)).rgb));
	float lumaUpLeft = RGBToLuma(ToneMapGammaCorrect(textureOffset(renderTarget, textureCoordinate, ivec2(-1, 1)).rgb));

	float lumaDownUp = lumaDown + lumaUp;
	float lumaLeftRight = lumaLeft + lumaRight;

	float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	float lumaDownCorners = lumaDownLeft + lumaDownRight;
	float lumaRightCorners = lumaDownRight + lumaUpRight;
	float lumaUpCorners = lumaUpRight + lumaUpLeft;


	float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs (-2.0 * lumaCenter + lumaDownUp ) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs (-2.0 * lumaCenter + lumaLeftRight ) * 2.0 + abs(-2.0 * lumaDown + lumaDownCorners);

	bool isHorizontal = (edgeHorizontal >= edgeVertical);

	float luma1 = isHorizontal ? lumaDown : lumaLeft;
	float luma2 = isHorizontal ? lumaUp : lumaRight;

	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;

	bool is1Steepest = abs(gradient1) >= abs(gradient2);

	float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

	float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

	
	float lumaLocalAverage = 0.0;

	if (is1Steepest)
	{
		stepLength = -stepLength;
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	}
	else
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);

	vec2 currentUV = textureCoordinate;

	if (isHorizontal)
		currentUV.y += stepLength * 0.5;
	else
		currentUV.x += stepLength * 0.5;


	vec2 offset = isHorizontal ? vec2(inverseScreenSize.x, 0.0) : vec2(0.0, inverseScreenSize.y);

	vec2 uv1 = currentUV - offset;
	vec2 uv2 = currentUV + offset;

	float lumaEnd1 = RGBToLuma(ToneMapGammaCorrect(texture(renderTarget, uv1).rgb));
	float lumaEnd2 = RGBToLuma(ToneMapGammaCorrect(texture(renderTarget, uv2).rgb));

	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;

	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;

	if(!reached1)
		uv1 -= offset;
	
	if(!reached2)
		uv2 += offset;

	if(!reachedBoth)
	{
		for(int i = 2; i < ITERATIONS; ++i)
		{
			if (!reached1)
			{
				lumaEnd1 = RGBToLuma(ToneMapGammaCorrect(texture(renderTarget, uv1).rgb));
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}

			if (!reached2)
			{
				lumaEnd2 = RGBToLuma(ToneMapGammaCorrect(texture(renderTarget, uv2).rgb));
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}

			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			if (!reached1)
				uv1 -= offset * Quality(i);

			if (!reached2)
				uv2 += offset * Quality(i);

			if (reachedBoth)
				break;
		}
	}

	float distance1 = isHorizontal ? (textureCoordinate.x - uv1.x) : (textureCoordinate.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - textureCoordinate.x) : (uv2.y - textureCoordinate.y);

	bool isDirection1 = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);

	float edgeThickness = (distance1 + distance2);

	float pixelOffset = -distanceFinal / edgeThickness + 0.5;

	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

	float finalOffset = correctVariation ? pixelOffset : 0.0;


	float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);

	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / deltaLuma, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;

	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

	finalOffset = max(finalOffset, subPixelOffsetFinal);

	vec2 finalUV = textureCoordinate;

	if(isHorizontal)
		finalUV.y += finalOffset * stepLength;
	else
		finalUV.x += finalOffset * stepLength;

	return ToneMapGammaCorrect(texture(renderTarget, finalUV).rgb);
}


float RGBToLuma(vec3 rgb)
{
	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

float Quality(int i)
{
	switch(i)
	{
		case 2: return 1.0;
		case 3: return 1.0;
		case 4: return 1.0;
		case 5: return 1.5;
		case 6: return 2.0;
		case 7: return 2.0;
		case 8: return 2.0;
		case 9: return 2.0;
		case 10: return 4.0;
		case 11: return 8.0;
	}
}