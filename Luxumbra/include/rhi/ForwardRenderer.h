#ifndef FORWARD_RENDER_PASS_H_INCLUDED
#define FORWARD_RENDER_PASS_H_INCLUDED

#include "Luxumbra.h"

#include "LuxVkImpl.h"

#include <vector>

namespace lux::rhi
{

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

		std::vector<VkImage> rtColorAttachmentImages;
		std::vector<VkDeviceMemory> rtColorAttachmentImageMemories;
		std::vector<VkImageView> rtColorAttachmentImageViews;

		VkImage rtDepthAttachmentImage;
		VkDeviceMemory rtDepthAttachmentMemory;
		VkImageView rtDepthAttachmentImageView;

		enum ForwardAttachmentBindPoints : uint32_t
		{
			FORWARD_SWAPCHAIN_ATTACHMENT_BIND_POINT = 0,
			FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT,
			FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT,
			FORWARD_ATTACHMENT_BIND_POINT_COUNT
		};

		enum ForwardSubpassess : uint32_t
		{
			FORWARD_SUBPASS_RENDER_TO_TARGET = 0,
			FORWARD_SUBPASS_COPY,
			FORWARD_SUBPASS_COUNT
		};
	};

}

#endif // FORWARD_RENDER_PASS_H_INCLUDED