#ifndef GRAPHIC_PIPELINE_H_INCLUDED
#define GRAPHIC_PIPELINE_H_INCLUDED

#include "Luxumbra.h"

#include <string>
#include <vector>

#include "rhi\LuxVkImpl.h"

namespace lux::rhi
{
	struct GraphicsPipelineCreateInfo
	{
		std::string binerayVertexFilePath;
		std::string binerayFragmentFilePath;
		VkPrimitiveTopology primitiveTopology;
		float viewportWidth;
		float viewportHeight;
		VkCullModeFlags rasterizerCullMode;
		VkFrontFace rasterizerFrontFace;
		bool enableDepthTest;
		bool enableDepthWrite;
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		std::vector<VkPushConstantRange> pushConstants;
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
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout descriptorSetLayout;
	};

} // namespace lux::rhi

#endif // GRAPHIC_PIPELINE_H_INCLUDED