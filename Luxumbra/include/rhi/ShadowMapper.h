#ifndef SHADOW_MAPPER_H_INCLUDED
#define SHADOW_MAPPER_H_INCLUDED

#include "Luxumbra.h"

#include "glm\glm.hpp"

#include "rhi\GraphicsPipeline.h"
#include "rhi\Buffer.h"
#include "rhi\Image.h"

#define DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE 2048
#define POINT_SHADOW_MAP_TEXTURE_SIZE 512

namespace lux::rhi
{

	struct ShadowMappingViewProjUniform
	{
		glm::mat4 viewProj;
	};

	struct ShadowMappingModelConstant
	{
		glm::mat4 model;
	};

	struct ShadowMapper
	{
		ShadowMapper() noexcept;
		ShadowMapper(const ShadowMapper&) = delete;
		ShadowMapper(ShadowMapper&&) = delete;

		~ShadowMapper() noexcept = default;

		const ShadowMapper& operator =(const ShadowMapper&) = delete;
		const ShadowMapper& operator =(ShadowMapper&&) = delete;

		VkRenderPass renderPass;

		GraphicsPipeline shadowMappingPipeline;

		VkDescriptorPool descriptorPool;

		Image directionalShadowMapIntermediate;
		Image dummyDirectionalShadowMap;
		VkFramebuffer directionalFramebuffer;
		std::vector<Image> directionalShadowMaps;
		std::vector<Buffer> directionalUniformBuffers;
		std::vector<VkDescriptorSet> directionalUniformBufferDescriptorSets;

		Image pointShadowMapIntermediate;
		Image dummyPointShadowMap;
		VkFramebuffer pointFramebuffer;
		std::vector<Image> pointShadowMaps;
		std::vector<Buffer> pointUniformBuffers;
		std::vector<VkDescriptorSet> pointUniformBufferDescriptorSets;
	};

} // namespace lux::rhi

#endif // SHADOW_MAPPER_H_INCLUDED