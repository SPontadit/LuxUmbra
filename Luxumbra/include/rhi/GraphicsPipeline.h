#ifndef GRAPHIC_PIPELINE_H_INCLUDED
#define GRAPHIC_PIPELINE_H_INCLUDED

#include "Luxumbra.h"

#include <string>
#include <vector>

#include "rhi\LuxVkImpl.h"
#include "Vertex.h"

namespace lux::rhi
{
	struct GraphicsPipelineCreateInfo
	{
		VkRenderPass renderPass;
		uint32_t subpassIndex;
		std::string binaryVertexFilePath;
		std::string binaryFragmentFilePath;
		std::string cacheFilePath;
		lux::VertexLayout vertexLayout;
		VkPrimitiveTopology primitiveTopology;
		float viewportWidth;
		float viewportHeight;
		VkCullModeFlags rasterizerCullMode;
		VkFrontFace rasterizerFrontFace;
		VkBool32 disableColorWriteMask;
		VkBool32 enableBlend;
		VkBool32 disableMSAA;
		VkBool32 enableDepthTest;
		VkBool32 enableDepthWrite;
		VkBool32 enableDepthBias;
		float depthBiasConstantFactor;
		float depthBiasSlopeFactor;
		VkCompareOp depthCompareOp;
		std::vector<VkDescriptorSetLayoutBinding> viewDescriptorSetLayoutBindings;
		std::vector<VkDescriptorSetLayoutBinding> materialDescriptorSetLayoutBindings;
		std::vector<VkDescriptorSetLayoutBinding> modelDescriptorSetLayoutBindings;
		std::vector<VkPushConstantRange> pushConstants;
		std::vector<VkDynamicState> dynamicStates;
	};

	struct GraphicsPipeline
	{
		GraphicsPipeline() noexcept;
		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&&) = delete;

		~GraphicsPipeline() noexcept = default;

		const GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
		const GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;


		VkPipeline pipeline;
		VkPipelineCache cache;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout viewDescriptorSetLayout;
		VkDescriptorSetLayout materialDescriptorSetLayout;
		VkDescriptorSetLayout modelDescriptorSetLayout;
	};

} // namespace lux::rhi

#endif // GRAPHIC_PIPELINE_H_INCLUDED