#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include "Luxumbra.h"

#include <string>

#include "rhi\LuxVkImpl.h"

namespace lux::rhi
{
	struct ImageCreateInfo
	{
		VkFormat format;
		uint32_t width;
		uint32_t height;
		uint32_t arrayLayers;
		uint32_t mipmapCount = 1;
		VkImageUsageFlags usage;
		uint32_t subresourceRangeLayerCount;
		VkImageAspectFlagBits subresourceRangeAspectMask;
		VkImageViewType imageViewType;
		void* imageData;
		uint64_t imageSize;
	};

	struct CubeMapCreateInfo
	{
		VkFormat format;
		uint32_t size;
		uint32_t mipmapCount = 1;
		std::string binaryVertexFilePath;
		std::string binaryFragmentFilePath;
		VkSampler sampler;
	};

	struct Image
	{
		Image() noexcept;
		Image(const Image&) = delete;
		Image(Image&&) = default;

		~Image() noexcept = default;

		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = delete;

		VkImage image;
		VkImageView imageView;
		VkDeviceMemory memory;
	};

} // namespace lux::rhi

#endif // IMAGE_H_INCLUDED