#include "rhi\RHI.h"

#include <array>

namespace lux::rhi
{
	ForwardRenderer::ForwardRenderer() noexcept
		: renderPass(VK_NULL_HANDLE), frameBuffers(0),
		rtColorAttachmentImages(0), rtColorAttachmentImageMemories(0), rtColorAttachmentImageViews(0),
		rtDepthAttachmentImage(VK_NULL_HANDLE), rtDepthAttachmentMemory(VK_NULL_HANDLE), rtDepthAttachmentImageView(VK_NULL_HANDLE)
	{

	}

	void RHI::InitForwardRenderPass() noexcept
	{
		VkAttachmentDescription swapchainAttachment = {};
		swapchainAttachment.format = swapchainImageFormat;
		swapchainAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		swapchainAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		swapchainAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		swapchainAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		swapchainAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		swapchainAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		swapchainAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription rtColorAttachment = {};
		rtColorAttachment.format = swapchainImageFormat;
		rtColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		rtColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription rtDepthAttachment = {};

		std::vector<VkFormat> depthAttachmentFormats{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
		rtDepthAttachment.format = FindSupportedImageFormat(depthAttachmentFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		ASSERT(rtDepthAttachment.format != VK_FORMAT_MAX_ENUM);

		rtDepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		rtDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtDepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtDepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtDepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtDepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference swapchainAttachmentRef = {};
		swapchainAttachmentRef.attachment = ForwardRenderer::FORWARD_SWAPCHAIN_ATTACHMENT_BIND_POINT;
		swapchainAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtColorAttachmentRef = {};
		rtColorAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT;
		rtColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference swapchainInputAttachmentRef = {};
		swapchainInputAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT;
		swapchainInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference rtDepthAttachmentRef = {};
		rtDepthAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT;
		rtColorAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription renderToTargetSubpass = {};
		renderToTargetSubpass.colorAttachmentCount = 1;
		renderToTargetSubpass.pColorAttachments = &rtColorAttachmentRef;
		renderToTargetSubpass.pDepthStencilAttachment = &rtDepthAttachmentRef;
		renderToTargetSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkSubpassDescription copySubpass = {};
		copySubpass.colorAttachmentCount = 1;
		copySubpass.pColorAttachments = &swapchainAttachmentRef;
		copySubpass.inputAttachmentCount = 1;
		copySubpass.pInputAttachments = &swapchainInputAttachmentRef;
		copySubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		std::array<VkAttachmentDescription, TO_SIZE_T(ForwardRenderer::FORWARD_ATTACHMENT_BIND_POINT_COUNT)> attachments{
			swapchainAttachment,
			rtColorAttachment,
			rtDepthAttachment
		};

		std::array<VkSubpassDescription, TO_SIZE_T(ForwardRenderer::FORWARD_SUBPASS_COUNT)> subpasses{
			renderToTargetSubpass,
			copySubpass
		};

		VkSubpassDependency subpassDependency = {};
		subpassDependency.dependencyFlags = 0;
		subpassDependency.srcSubpass = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency.dstSubpass = ForwardRenderer::FORWARD_SUBPASS_COPY;
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpassDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		VkRenderPassCreateInfo renderPassCI = {};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = ForwardRenderer::FORWARD_ATTACHMENT_BIND_POINT_COUNT;
		renderPassCI.pAttachments = attachments.data();
		renderPassCI.subpassCount = ForwardRenderer::FORWARD_SUBPASS_COUNT;
		renderPassCI.pSubpasses = subpasses.data();
		renderPassCI.dependencyCount = 1;
		renderPassCI.pDependencies = &subpassDependency;

		CHECK_VK(vkCreateRenderPass(device, &renderPassCI, nullptr, &forward.renderPass));
	}

	void RHI::InitForwardFramebuffers() noexcept
	{
		// TODO: Use CreateImage
		// Used by color & depth attachments
		VkMemoryRequirements memoryRequirements;
		VkMemoryAllocateInfo rtAttachmentImageAI = {};
		rtAttachmentImageAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		// Color attachment

		VkImageCreateInfo rtColorAttachmentImageCI = {};
		rtColorAttachmentImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		rtColorAttachmentImageCI.imageType = VK_IMAGE_TYPE_2D;
		rtColorAttachmentImageCI.format = swapchainImageFormat;
		rtColorAttachmentImageCI.extent = { swapchainExtent.width, swapchainExtent.height, 1 };
		rtColorAttachmentImageCI.mipLevels = 1;
		rtColorAttachmentImageCI.arrayLayers = 1;
		rtColorAttachmentImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		rtColorAttachmentImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		rtColorAttachmentImageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		rtColorAttachmentImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		rtColorAttachmentImageCI.queueFamilyIndexCount = 1;
		rtColorAttachmentImageCI.pQueueFamilyIndices = &graphicsQueueIndex;
		rtColorAttachmentImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageViewCreateInfo rtColorAttachmentImageViewCI = {};
		rtColorAttachmentImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		rtColorAttachmentImageViewCI.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		rtColorAttachmentImageViewCI.format = swapchainImageFormat;
		rtColorAttachmentImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		rtColorAttachmentImageViewCI.subresourceRange = swapchainImageSubresourceRange;


		forward.rtColorAttachmentImages.resize(TO_SIZE_T(swapchainImageCount));
		forward.rtColorAttachmentImageMemories.resize(TO_SIZE_T(swapchainImageCount));
		forward.rtColorAttachmentImageViews.resize(TO_SIZE_T(swapchainImageCount));

		for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			VkImage* rtColorAttachmentImage = &forward.rtColorAttachmentImages[TO_SIZE_T(i)];
			VkDeviceMemory* rtColorAttachmentImageMemory = &forward.rtColorAttachmentImageMemories[TO_SIZE_T(i)];
			VkImageView* rtColorAttachmentImageView = &forward.rtColorAttachmentImageViews[TO_SIZE_T(i)];

			CHECK_VK(vkCreateImage(device, &rtColorAttachmentImageCI, nullptr, rtColorAttachmentImage));

			vkGetImageMemoryRequirements(device, *rtColorAttachmentImage, &memoryRequirements);
			rtAttachmentImageAI.allocationSize = memoryRequirements.size;
			rtAttachmentImageAI.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			CHECK_VK(vkAllocateMemory(device, &rtAttachmentImageAI, nullptr, rtColorAttachmentImageMemory));
			CHECK_VK(vkBindImageMemory(device, *rtColorAttachmentImage, *rtColorAttachmentImageMemory, 0));

			rtColorAttachmentImageViewCI.image = *rtColorAttachmentImage;

			CHECK_VK(vkCreateImageView(device, &rtColorAttachmentImageViewCI, nullptr, rtColorAttachmentImageView));
		}

		// Depth attachment

		std::vector<VkFormat> depthAttachmentFormats{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
		VkFormat depthImageFormat = FindSupportedImageFormat(depthAttachmentFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		ASSERT(depthImageFormat != VK_FORMAT_MAX_ENUM);

		VkImageCreateInfo rtDepthAttachmentImageCI = {};
		rtDepthAttachmentImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		rtDepthAttachmentImageCI.imageType = VK_IMAGE_TYPE_2D;
		rtDepthAttachmentImageCI.format = depthImageFormat;
		rtDepthAttachmentImageCI.extent = { swapchainExtent.width, swapchainExtent.height, 1 };
		rtDepthAttachmentImageCI.mipLevels = 1;
		rtDepthAttachmentImageCI.arrayLayers = 1;
		rtDepthAttachmentImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		rtDepthAttachmentImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		rtDepthAttachmentImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		rtDepthAttachmentImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		rtDepthAttachmentImageCI.queueFamilyIndexCount = 1;
		rtDepthAttachmentImageCI.pQueueFamilyIndices = &graphicsQueueIndex;
		rtDepthAttachmentImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		CHECK_VK(vkCreateImage(device, &rtDepthAttachmentImageCI, nullptr, &forward.rtDepthAttachmentImage));

		vkGetImageMemoryRequirements(device, forward.rtDepthAttachmentImage, &memoryRequirements);
		rtAttachmentImageAI.allocationSize = memoryRequirements.size;
		rtAttachmentImageAI.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		CHECK_VK(vkAllocateMemory(device, &rtAttachmentImageAI, nullptr, &forward.rtDepthAttachmentMemory));
		CHECK_VK(vkBindImageMemory(device, forward.rtDepthAttachmentImage, forward.rtDepthAttachmentMemory, 0));

		VkImageViewCreateInfo rtDepthAttachmentImageViewCI = {};
		rtDepthAttachmentImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		rtDepthAttachmentImageViewCI.image = forward.rtDepthAttachmentImage;
		rtDepthAttachmentImageViewCI.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		rtDepthAttachmentImageViewCI.format = depthImageFormat;
		rtDepthAttachmentImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		rtDepthAttachmentImageViewCI.subresourceRange = swapchainImageSubresourceRange;
		rtDepthAttachmentImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		CHECK_VK(vkCreateImageView(device, &rtDepthAttachmentImageViewCI, nullptr, &forward.rtDepthAttachmentImageView));

		CommandTransitionImageLayout(forward.rtDepthAttachmentImage, depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		// Framebuffers

		std::array<VkImageView, TO_SIZE_T(ForwardRenderer::FORWARD_ATTACHMENT_BIND_POINT_COUNT)> attachments { VK_NULL_HANDLE };

		VkFramebufferCreateInfo framebufferCI = {};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = forward.renderPass;
		framebufferCI.width = swapchainExtent.width;
		framebufferCI.height = swapchainExtent.height;
		framebufferCI.layers = 1;
		framebufferCI.attachmentCount = ForwardRenderer::FORWARD_ATTACHMENT_BIND_POINT_COUNT;
		framebufferCI.pAttachments = attachments.data();

		forward.frameBuffers.resize(TO_SIZE_T(swapchainImageCount));

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_SWAPCHAIN_ATTACHMENT_BIND_POINT)] = swapchainImageViews[i];
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT)] = forward.rtColorAttachmentImageViews[i];
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT)] = forward.rtDepthAttachmentImageView;

			CHECK_VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &forward.frameBuffers[i]));
		}


		// Fences and semaphores
		VkSemaphoreCreateInfo rtSemaphoreCI = {};
		rtSemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo rtFenceCI = {};
		rtFenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		rtFenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		forward.presentSemaphores.resize(TO_SIZE_T(swapchainImageCount));
		forward.acquireSemaphores.resize(TO_SIZE_T(swapchainImageCount));
		forward.fences.resize(TO_SIZE_T(swapchainImageCount));

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			CHECK_VK(vkCreateSemaphore(device, &rtSemaphoreCI, nullptr, &forward.presentSemaphores[i]));
			CHECK_VK(vkCreateSemaphore(device, &rtSemaphoreCI, nullptr, &forward.acquireSemaphores[i]));

			CHECK_VK(vkCreateFence(device, &rtFenceCI, nullptr, &forward.fences[i]));
		}
	}
} // namespace lux::rhi