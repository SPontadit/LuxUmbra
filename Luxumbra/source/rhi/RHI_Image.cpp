#include "rhi\RHI.h"

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
		imageCI.arrayLayers = 1;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = luxImageCI.usage;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCI.queueFamilyIndexCount = 1;
		imageCI.pQueueFamilyIndices = &graphicsQueueIndex;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		CHECK_VK(vkCreateImage(device, &imageCI, nullptr, &image.image));
	
		// TODO: Use staging buffer
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
	
		
		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageViewCI.image = image.image;
		imageViewCI.format = luxImageCI.format;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.subresourceRange.aspectMask = luxImageCI.subresourceRangeAspectMask;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
	
		CHECK_VK(vkCreateImageView(device, &imageViewCI, nullptr, &image.imageView));
	}

} // namespace lux::rhi