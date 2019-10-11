#include "rhi\RHI.h"

#include <array>

#include "Utility.h"
#include "Vertex.h"
#include "rhi\LuxVkImpl.h"

namespace lux::rhi
{

	GraphicsPipeline::GraphicsPipeline() noexcept
		: pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE), descriptorSetLayout(VK_NULL_HANDLE)
	{
	
	}

	void RHI::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& luxGraphicsPipelineCI, GraphicsPipeline& graphicsPipeline) noexcept
	{
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		CreateShaderModule(luxGraphicsPipelineCI.binerayVertexFilePath, VK_SHADER_STAGE_VERTEX_BIT, &vertexShaderModule);
		CreateShaderModule(luxGraphicsPipelineCI.binerayFragmentFilePath, VK_SHADER_STAGE_FRAGMENT_BIT, &fragmentShaderModule);

		VkPipelineShaderStageCreateInfo vertexStageCI = {};
		vertexStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexStageCI.module = vertexShaderModule;
		vertexStageCI.pName = "main";

		VkPipelineShaderStageCreateInfo fragmentStageCI = {};
		vertexStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		vertexStageCI.module = fragmentShaderModule;
		vertexStageCI.pName = "main";

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertexStageCI, fragmentStageCI };


		VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
		std::array<VkVertexInputAttributeDescription, 3> attributesDescription = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {};
		vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCI.vertexBindingDescriptionCount = 1;
		vertexInputStateCI.pVertexBindingDescriptions = &bindingDescription;
		vertexInputStateCI.vertexAttributeDescriptionCount = TO_UINT32_T(attributesDescription.size());
		vertexInputStateCI.pVertexAttributeDescriptions = attributesDescription.data();


		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = {};
		inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCI.topology = luxGraphicsPipelineCI.primitiveTopology;
		inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;


		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = luxGraphicsPipelineCI.viewportWidth;
		viewport.height = luxGraphicsPipelineCI.viewportHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent.width = TO_UINT32_T(luxGraphicsPipelineCI.viewportWidth);
		scissor.extent.height = TO_UINT32_T(luxGraphicsPipelineCI.viewportHeight);

		VkPipelineViewportStateCreateInfo viewportStateCI = {};
		viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCI.viewportCount = 1;
		viewportStateCI.pViewports = &viewport;
		viewportStateCI.scissorCount = 1;
		viewportStateCI.pScissors = &scissor;


		VkPipelineRasterizationStateCreateInfo rasterizerStateCI = {};
		rasterizerStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerStateCI.depthClampEnable = VK_FALSE;
		rasterizerStateCI.rasterizerDiscardEnable = VK_FALSE;
		rasterizerStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerStateCI.lineWidth = 1.0f;
		rasterizerStateCI.cullMode = luxGraphicsPipelineCI.rasterizerCullMode;
		rasterizerStateCI.frontFace = luxGraphicsPipelineCI.rasterizerFrontFace;
		rasterizerStateCI.depthBiasEnable = VK_FALSE;
		rasterizerStateCI.depthBiasConstantFactor = 0.0f;
		rasterizerStateCI.depthBiasClamp = 0.0f;
		rasterizerStateCI.depthBiasSlopeFactor = 0.0f;


		VkPipelineMultisampleStateCreateInfo multisampleStateCI = {};
		multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCI.sampleShadingEnable = VK_FALSE;
		multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateCI.minSampleShading = 1.0f;
		multisampleStateCI.pSampleMask = nullptr;
		multisampleStateCI.alphaToCoverageEnable = VK_FALSE;
		multisampleStateCI.alphaToOneEnable = VK_FALSE;


		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI = {};
		colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCI.logicOpEnable = VK_FALSE;
		colorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateCI.attachmentCount = 1;
		colorBlendStateCI.pAttachments = &colorBlendAttachment;
		colorBlendStateCI.blendConstants[0] = 0.0f;
		colorBlendStateCI.blendConstants[1] = 0.0f;
		colorBlendStateCI.blendConstants[2] = 0.0f;
		colorBlendStateCI.blendConstants[3] = 0.0f;


		std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

		VkPipelineDynamicStateCreateInfo dynamicStateCI = {};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.dynamicStateCount = TO_UINT32_T(dynamicStates.size());
		dynamicStateCI.pDynamicStates = dynamicStates.data();


		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
		depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.depthTestEnable = luxGraphicsPipelineCI.enableDepthTest;
		depthStencilStateInfo.depthWriteEnable = luxGraphicsPipelineCI.enableDepthWrite;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;


		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = {};
		descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCI.bindingCount = TO_UINT32_T(luxGraphicsPipelineCI.descriptorSetLayoutBindings.size());
		descriptorSetLayoutCI.pBindings = luxGraphicsPipelineCI.descriptorSetLayoutBindings.data();

		CHECK_VK(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &graphicsPipeline.descriptorSetLayout));


		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &graphicsPipeline.descriptorSetLayout;
		pipelineLayoutCI.pushConstantRangeCount = TO_UINT32_T(luxGraphicsPipelineCI.pushConstants.size());
		pipelineLayoutCI.pPushConstantRanges = luxGraphicsPipelineCI.pushConstants.data();

		CHECK_VK(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &graphicsPipeline.pipelineLayout));
		

		VkGraphicsPipelineCreateInfo pipelineCI = {};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.stageCount = TO_UINT32_T(shaderStages.size());
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.pVertexInputState = &vertexInputStateCI;
		pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pRasterizationState = &rasterizerStateCI;
		pipelineCI.pMultisampleState = &multisampleStateCI;
		pipelineCI.pDepthStencilState = &depthStencilStateInfo;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.layout = graphicsPipeline.pipelineLayout;

		// TODO: Complete renderpass and subpass
		//pipelineCI.renderPass = ???
		//pipelineCI.subpass = ???

		pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCI.basePipelineIndex = -1;

		CHECK_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &graphicsPipeline.pipeline));


		vkDestroyShaderModule(device, vertexShaderModule, nullptr);
		vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
	}

	void RHI::CreateShaderModule(const std::string& binaryFilePath, VkShaderStageFlagBits pipelineStage, VkShaderModule* shaderModule) const noexcept
	{
		std::vector<char> shaderCode = lux::utility::ReadFile(binaryFilePath);

		VkShaderModuleCreateInfo shaderModuleCI = {};
		shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCI.codeSize = shaderCode.size();
		shaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		CHECK_VK(vkCreateShaderModule(device, &shaderModuleCI, nullptr, shaderModule));
	}

	void RHI::DestroyGraphicsPipeline(GraphicsPipeline& graphicsPipeline) noexcept
	{
		vkDestroyDescriptorSetLayout(device, graphicsPipeline.descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, graphicsPipeline.pipelineLayout, nullptr);
		vkDestroyPipeline(device, graphicsPipeline.pipeline, nullptr);
	}


} // namespace lux::rhi