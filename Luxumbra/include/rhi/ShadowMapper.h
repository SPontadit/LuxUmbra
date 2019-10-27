#ifndef SHADOW_MAPPER_H_INCLUDED
#define SHADOW_MAPPER_H_INCLUDED

#include "Luxumbra.h"

#include "glm\glm.hpp"

#include "rhi\GraphicsPipeline.h"
#include "rhi\Buffer.h"
#include "rhi\Image.h"

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

		std::vector<Image> shadowMaps;
		std::vector<VkFramebuffer> framebuffers;

		GraphicsPipeline directionalShadowMappingPipeline;

		Buffer viewProjUniformBuffer;

		VkDescriptorPool descriptorPool;
		VkDescriptorSet viewProjUniformBufferDescriptorSet;
	};

} // namespace lux::rhi

#endif // SHADOW_MAPPER_H_INCLUDED