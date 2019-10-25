#ifndef SHADOW_MAPPING_PIPELINE_H_INCLUDED
#define SHADOW_MAPPING_PIPELINE_H_INCLUDED

#include "Luxumbra.h"

#include "rhi\LuxVkImpl.h"

namespace lux::rhi
{

	using namespace lux;

	struct ShadowMappingPipeline
	{
		ShadowMappingPipeline() noexcept;
		ShadowMappingPipeline(const ShadowMappingPipeline&) = delete;
		ShadowMappingPipeline(ShadowMappingPipeline&&) = delete;

		~ShadowMappingPipeline() noexcept;

		const ShadowMappingPipeline& operator =(const ShadowMappingPipeline&) = delete;
		const ShadowMappingPipeline& operator =(ShadowMappingPipeline&&) = delete;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout vpUBODescriptorSetLayout;
		VkDescriptorSetLayout modelDescriptorSetLayout;
	};

} // namespace lux::rhi

#endif // SHADOW_MAPPING_PIPELINE_H_INCLUDED