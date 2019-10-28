#include "rhi\RHI.h"

namespace lux::rhi
{
	ComputePipeline::ComputePipeline() noexcept
		: pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE), descriptorSetLayout(VK_NULL_HANDLE)
	{

	}

	void RHI::CreateComputePipeline(const ComputePipelineCreateInfo& luxComputePipelineCI, ComputePipeline& computePipeline) noexcept
	{
		VkShaderModule computeShaderModule = VK_NULL_HANDLE;
		CreateShaderModule(luxComputePipelineCI.binaryComputeFilePath, &computeShaderModule);

		VkPipelineShaderStageCreateInfo computeStageCI = {};
		computeStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeStageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeStageCI.module = computeShaderModule;
		computeStageCI.pName = "main";

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = {};
		descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCI.bindingCount = TO_UINT32_T(luxComputePipelineCI.descriptorSetLayoutBindings.size());
		descriptorSetLayoutCI.pBindings = luxComputePipelineCI.descriptorSetLayoutBindings.data();

		CHECK_VK(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &computePipeline.descriptorSetLayout));

		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &computePipeline.descriptorSetLayout;
		pipelineLayoutCI.pushConstantRangeCount = TO_UINT32_T(luxComputePipelineCI.pushConstants.size());
		pipelineLayoutCI.pPushConstantRanges = luxComputePipelineCI.pushConstants.data();

		CHECK_VK(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &computePipeline.pipelineLayout));

		VkComputePipelineCreateInfo computePipelineCI = {};
		computePipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCI.stage = computeStageCI;
		computePipelineCI.layout = computePipeline.pipelineLayout;

		CHECK_VK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCI, nullptr, &computePipeline.pipeline));
	
		vkDestroyShaderModule(device, computeShaderModule, nullptr);
	}

	void RHI::DestroyComputePipeline(ComputePipeline& computePipeline) noexcept
	{
		vkDestroyPipeline(device, computePipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(device, computePipeline.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, computePipeline.descriptorSetLayout, nullptr);
	}

} // namespace lux::rhi