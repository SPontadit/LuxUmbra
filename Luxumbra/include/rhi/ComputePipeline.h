#ifndef COMPUTE_PIPELINE_H_INCLUDED
#define COMPUTE_PIPELINE_H_INCLUDED

#include "Luxumbra.h"

#include "rhi\LuxVkImpl.h"


namespace lux::rhi
{
	struct ComputePipelineCreateInfo
	{
		std::string binaryComputeFilePath;
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		std::vector<VkPushConstantRange> pushConstants;
	};

	struct ComputePipeline
	{
		ComputePipeline() noexcept;
		// TODO: TMP FOR IBL DESTRUCTION
		ComputePipeline(const ComputePipeline&) = default;
		ComputePipeline(ComputePipeline&&) = default;

		~ComputePipeline() noexcept = default;

		const ComputePipeline& operator=(const ComputePipeline&) = delete;
		const ComputePipeline& operator=(ComputePipeline&&) = delete;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout descriptorSetLayout;
	};

} // namespace lux::rhi

#endif // COMPUTE_PIPELINE_H_INCLUDED