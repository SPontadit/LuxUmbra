#ifndef FORWARD_RENDER_PASS_H_INCLUDED
#define FORWARD_RENDER_PASS_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "glm\glm.hpp"

#include "LuxVkImpl.h"
#include "resource\Mesh.h"


namespace lux::rhi
{
	struct RtViewProjUniform
	{
		glm::mat4 projection;
		glm::mat4 view;
	};

	struct RtModelConstant
	{
		glm::mat4 model;
	};

	struct ForwardRenderer
	{
		ForwardRenderer() noexcept;
		ForwardRenderer(const ForwardRenderer&) = delete;
		ForwardRenderer(ForwardRenderer&&) = delete;

		~ForwardRenderer() noexcept = default;

		const ForwardRenderer& operator=(const ForwardRenderer&) = delete;
		const ForwardRenderer& operator=(ForwardRenderer&&) = delete;

		VkRenderPass renderPass;
		std::vector<VkFramebuffer> frameBuffers;
		VkDescriptorPool descriptorPool;

		GraphicsPipeline blitGraphicsPipeline;
		std::vector<VkDescriptorSet> blitDescriptorSets;

		GraphicsPipeline rtGraphicsPipeline;
		GraphicsPipeline rtCutoutGraphicsPipeline;
		GraphicsPipeline rtTransparentBackGraphicsPipeline;
		GraphicsPipeline rtTransparentFrontGraphicsPipeline;
		std::vector<VkDescriptorSet> rtViewDescriptorSets;
		std::vector<VkDescriptorSet> rtModelDescriptorSets;

		GraphicsPipeline envMapGraphicsPipeline;
		std::vector<VkDescriptorSet> envMapViewDescriptorSets;

		RtModelConstant modelConstant;
		std::vector<Buffer> viewProjUniformBuffers;

		std::vector<VkImage> rtColorAttachmentImages;
		std::vector<VkImageView> rtColorAttachmentImageViews;
		std::vector<VkDeviceMemory> rtColorAttachmentImageMemories;

		VkImage rtResolveColorAttachmentImage;
		VkImageView rtResolveColorAttachmentImageView;
		VkDeviceMemory rtResolveColorAttachmentMemory;

		VkImage rtDepthAttachmentImage;
		VkImageView rtDepthAttachmentImageView;
		VkDeviceMemory rtDepthAttachmentMemory;

		VkSampler sampler;
		VkSampler cubemapSampler;

		enum ForwardAttachmentBindPoints : uint32_t
		{
			FORWARD_SWAPCHAIN_ATTACHMENT_BIND_POINT = 0,
			FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT,
			FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT,
			FORWARD_RT_RESOLVE_COLOR_ATTACHMENT_BIND_POINT,
			FORWARD_ATTACHMENT_BIND_POINT_COUNT
		};

		enum ForwardSubpassess : uint32_t
		{
			FORWARD_SUBPASS_RENDER_TO_TARGET = 0,
			FORWARD_SUBPASS_COPY,
			FORWARD_SUBPASS_COUNT
		};

		enum ForwardDescriptorSetLayouts : uint32_t
		{
			FORWARD_VIEW_DESCRIPTOR_SET_LAYOUT = 0,
			FORWARD_MATERIAL_DESCRIPTOR_SET_LAYOUT,
			FORWARD_MODEL_DESCRIPTOR_SET_LAYOUT,
			FORWARD_DESCRIPTOR_SET_LAYOUT_COUNT
		};
	};

} // namespace lux::rhi

#endif // FORWARD_RENDER_PASS_H_INCLUDED