#include "rhi\RHI.h"

#include <array>

#include "utility\Utility.h"
#include "Vertex.h"
#include "rhi\LuxVkImpl.h"

namespace lux::rhi
{

	GraphicsPipeline::GraphicsPipeline() noexcept
		: pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE), 
		viewDescriptorSetLayout(VK_NULL_HANDLE), materialDescriptorSetLayout(VK_NULL_HANDLE), modelDescriptorSetLayout(VK_NULL_HANDLE)
	{
	
	}

	void RHI::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& luxGraphicsPipelineCI, GraphicsPipeline& graphicsPipeline) noexcept
	{
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		CreateShaderModule(luxGraphicsPipelineCI.binaryVertexFilePath, &vertexShaderModule);
		CreateShaderModule(luxGraphicsPipelineCI.binaryFragmentFilePath, &fragmentShaderModule);

		VkPipelineShaderStageCreateInfo vertexStageCI = {};
		vertexStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexStageCI.module = vertexShaderModule;
		vertexStageCI.pName = "main";

		VkPipelineShaderStageCreateInfo fragmentStageCI = {};
		fragmentStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentStageCI.module = fragmentShaderModule;
		fragmentStageCI.pName = "main";

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertexStageCI, fragmentStageCI };


		VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
		std::array<VkVertexInputAttributeDescription, 3> attributesDescription = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {};
		vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		if (luxGraphicsPipelineCI.emptyVertexInput)
		{
			vertexInputStateCI.vertexBindingDescriptionCount = 0;
			vertexInputStateCI.pVertexBindingDescriptions = nullptr;
			vertexInputStateCI.vertexAttributeDescriptionCount = 0;
			vertexInputStateCI.pVertexAttributeDescriptions = nullptr;
		}
		else
		{
			vertexInputStateCI.vertexBindingDescriptionCount = 1;
			vertexInputStateCI.pVertexBindingDescriptions = &bindingDescription;
			vertexInputStateCI.vertexAttributeDescriptionCount = TO_UINT32_T(attributesDescription.size());
			vertexInputStateCI.pVertexAttributeDescriptions = attributesDescription.data();
		}


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


		VkPipelineDynamicStateCreateInfo dynamicStateCI = {};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.dynamicStateCount = TO_UINT32_T(luxGraphicsPipelineCI.dynamicStates.size());
		dynamicStateCI.pDynamicStates = luxGraphicsPipelineCI.dynamicStates.data();


		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = {};
		depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateCI.depthTestEnable = luxGraphicsPipelineCI.enableDepthTest;
		depthStencilStateCI.depthWriteEnable = luxGraphicsPipelineCI.enableDepthWrite;
		depthStencilStateCI.depthCompareOp = luxGraphicsPipelineCI.depthCompareOp;
		depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;


		// Descriptor Set Layouts
		VkDescriptorSetLayoutCreateInfo viewDescriptorSetLayoutCI = {};
		viewDescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		viewDescriptorSetLayoutCI.bindingCount = TO_UINT32_T(luxGraphicsPipelineCI.viewDescriptorSetLayoutBindings.size());
		viewDescriptorSetLayoutCI.pBindings = luxGraphicsPipelineCI.viewDescriptorSetLayoutBindings.data();

		CHECK_VK(vkCreateDescriptorSetLayout(device, &viewDescriptorSetLayoutCI, nullptr, &graphicsPipeline.viewDescriptorSetLayout));

		VkDescriptorSetLayoutCreateInfo materialDescriptorSetLayoutCI = {};
		materialDescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		materialDescriptorSetLayoutCI.bindingCount = TO_UINT32_T(luxGraphicsPipelineCI.materialDescriptorSetLayoutBindings.size());
		materialDescriptorSetLayoutCI.pBindings = luxGraphicsPipelineCI.materialDescriptorSetLayoutBindings.data();

		CHECK_VK(vkCreateDescriptorSetLayout(device, &materialDescriptorSetLayoutCI, nullptr, &graphicsPipeline.materialDescriptorSetLayout));

		VkDescriptorSetLayoutCreateInfo modelDescriptorSetLayoutCI = {};
		modelDescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		modelDescriptorSetLayoutCI.bindingCount = TO_UINT32_T(luxGraphicsPipelineCI.modelDescriptorSetLayoutBindings.size());
		modelDescriptorSetLayoutCI.pBindings = luxGraphicsPipelineCI.modelDescriptorSetLayoutBindings.data();

		CHECK_VK(vkCreateDescriptorSetLayout(device, &modelDescriptorSetLayoutCI, nullptr, &graphicsPipeline.modelDescriptorSetLayout));

		std::array<VkDescriptorSetLayout, ForwardRenderer::FORWARD_DESCRIPTOR_SET_LAYOUT_COUNT> descriptorSetLayouts =
		{
			graphicsPipeline.viewDescriptorSetLayout,
			graphicsPipeline.materialDescriptorSetLayout,
			graphicsPipeline.modelDescriptorSetLayout
		};

		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = TO_UINT32_T(descriptorSetLayouts.size());
		pipelineLayoutCI.pSetLayouts = descriptorSetLayouts.data();
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
		pipelineCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.layout = graphicsPipeline.pipelineLayout;
		pipelineCI.renderPass = luxGraphicsPipelineCI.renderPass;
		pipelineCI.subpass = luxGraphicsPipelineCI.subpassIndex;
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCI.basePipelineIndex = -1;

		CHECK_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &graphicsPipeline.pipeline));
	
		vkDestroyShaderModule(device, vertexShaderModule, nullptr);
		vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
	}

	void RHI::CreateShaderModule(const std::string& binaryFilePath, VkShaderModule* shaderModule) const noexcept
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
		vkDestroyPipeline(device, graphicsPipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(device, graphicsPipeline.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, graphicsPipeline.viewDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, graphicsPipeline.materialDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, graphicsPipeline.modelDescriptorSetLayout, nullptr);
	}

} // namespace lux::rhi