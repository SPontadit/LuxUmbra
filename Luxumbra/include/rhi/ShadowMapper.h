#ifndef SHADOW_MAPPER_H_INCLUDED
#define SHADOW_MAPPER_H_INCLUDED

#include "Luxumbra.h"

#include "glm\glm.hpp"

#include "rhi\GraphicsPipeline.h"
#include "rhi\Buffer.h"
#include "rhi\Image.h"

namespace lux::rhi
{

	struct DirectionalShadowMappingViewProjUniform
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

		GraphicsPipeline directionalShadowMappingPipeline;

		VkDescriptorPool descriptorPool;

		std::vector<Image> directionalShadowMaps;
		std::vector<VkFramebuffer> directionalFramebuffers;
		std::vector<Buffer> directionalUniformBuffers;
		std::vector<VkDescriptorSet> directionalUniformBufferDescriptorSets;

		std::vector<Image> pointShadowMaps;
		std::vector<VkFramebuffer> pointFramebuffers;
		std::vector<Buffer> pointUniformBuffers;
		std::vector<VkDescriptorSet> pointUniformBufferDescriptorSets;
	};

} // namespace lux::rhi

#endif // SHADOW_MAPPER_H_INCLUDED