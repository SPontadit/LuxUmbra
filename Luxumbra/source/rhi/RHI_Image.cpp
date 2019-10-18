#include "rhi\RHI.h"

#include <array>

namespace lux::rhi
{

	Image::Image() noexcept
		: image(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE), memory(VK_NULL_HANDLE)
	{

	}

	void RHI::CreateImage(const ImageCreateInfo& luxImageCI, Image& image) noexcept
	{
		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = luxImageCI.format;
		imageCI.extent.width = luxImageCI.width;
		imageCI.extent.height = luxImageCI.height;
		imageCI.extent.depth = 1;
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = luxImageCI.arrayLayers;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = luxImageCI.usage;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCI.queueFamilyIndexCount = 1;
		imageCI.pQueueFamilyIndices = &graphicsQueueIndex;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (luxImageCI.arrayLayers != 1)
		{
			imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		CHECK_VK(vkCreateImage(device, &imageCI, nullptr, &image.image));
	
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, image.image, &memoryRequirements);
		uint32_t memoryType = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		ASSERT(memoryType != UINT32_MAX);

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.memoryTypeIndex = memoryType;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;

		CHECK_VK(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &image.memory));
		CHECK_VK(vkBindImageMemory(device, image.image, image.memory, 0));
	
		if (luxImageCI.imageData != nullptr)
			FillImage(luxImageCI, image);
		
		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = image.image;
		imageViewCI.format = luxImageCI.format;
		imageViewCI.viewType = luxImageCI.imageViewType;
		imageViewCI.subresourceRange.aspectMask = luxImageCI.subresourceRangeAspectMask;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.layerCount = luxImageCI.subresourceRangeLayerCount;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
	
		CHECK_VK(vkCreateImageView(device, &imageViewCI, nullptr, &image.imageView));
	}

	void RHI::FillImage(const ImageCreateInfo& luxImageCI, Image& image) noexcept
	{
		BufferCreateInfo stagingBufferCI = {};
		stagingBufferCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferCI.size = luxImageCI.imageSize;
		stagingBufferCI.data = luxImageCI.imageData;
		stagingBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		stagingBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		Buffer stagingBuffer;
		CreateBuffer(stagingBufferCI, stagingBuffer);

		CommandTransitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy bufferImageCopy = {};
		bufferImageCopy.bufferOffset = 0;
		bufferImageCopy.bufferImageHeight = 0;
		bufferImageCopy.bufferRowLength = 0;
		bufferImageCopy.imageSubresource.aspectMask = luxImageCI.subresourceRangeAspectMask;
		bufferImageCopy.imageSubresource.baseArrayLayer = 0;
		bufferImageCopy.imageSubresource.layerCount = luxImageCI.subresourceRangeLayerCount;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageOffset = { 0, 0, 0 };
		bufferImageCopy.imageExtent.width = luxImageCI.width;
		bufferImageCopy.imageExtent.height = luxImageCI.height;
		bufferImageCopy.imageExtent.depth = 1;

		VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

		EndSingleTimeCommandBuffer(commandBuffer);

		CommandTransitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
		DestroyBuffer(stagingBuffer);
	}

	void RHI::GenerateCubemap(const ImageCreateInfo& luxImageCI, Image& image) noexcept
	{
		struct OffscreenResource
		{
			VkRenderPass renderPass;
			VkImage image;
			VkImageView imageView;
			VkDeviceMemory memory;
			VkFramebuffer framebuffer;
		} offscreen;

		// Attachment
		VkAttachmentDescription attachmentDescription = {};
		attachmentDescription.format = luxImageCI.format;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentReference = {};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		// Subpass
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentReference;

		VkSubpassDependency subpassDependency_0 = {};
		subpassDependency_0.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependency_0.dstSubpass = 0;
		subpassDependency_0.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependency_0.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency_0.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependency_0.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency_0.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkSubpassDependency subpassDependency_1 = {};
		subpassDependency_1.srcSubpass = 0;
		subpassDependency_1.dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependency_1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency_1.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependency_1.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency_1.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependency_1.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		std::array<VkSubpassDependency, 2> subpassDependencies { subpassDependency_0, subpassDependency_1 };
	

		VkRenderPassCreateInfo renderPassCI = {};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &attachmentDescription;
		renderPassCI.dependencyCount = TO_UINT32_T(subpassDependencies.size());
		renderPassCI.pDependencies = subpassDependencies.data();
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;

		CHECK_VK(vkCreateRenderPass(device, &renderPassCI, nullptr, &offscreen.renderPass));

		uint32_t dimension = 1024;

		ImageCreateInfo imageCI = {};
		imageCI.format = luxImageCI.format;
		imageCI.width = dimension;
		imageCI.height = dimension;


		vkDestroyFramebuffer(device, offscreen.framebuffer, nullptr);
		vkDestroyImageView(device, offscreen.imageView, nullptr);
		vkDestroyImage(device, offscreen.image, nullptr);
		vkFreeMemory(device, offscreen.memory, nullptr);
		vkDestroyRenderPass(device, offscreen.renderPass, nullptr);
	}

	void RHI::DestroyImage(Image& image) noexcept
	{
		vkDestroyImageView(device, image.imageView, nullptr);
		vkDestroyImage(device, image.image, nullptr);
		vkFreeMemory(device, image.memory, nullptr);
	}


} // namespace lux::rhi