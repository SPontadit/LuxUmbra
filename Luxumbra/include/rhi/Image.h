#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include "Luxumbra.h"

#include "rhi\LuxVkImpl.h"

namespace lux::rhi
{
	struct ImageCreateInfo
	{
		VkFormat format;
		uint32_t width;
		uint32_t height;
		VkImageUsageFlags usage;
		VkImageAspectFlagBits subresourceRangeAspectMask;
		void* imageData;
		uint64_t imageSize;
	};


	struct Image
	{
		Image() noexcept;
		Image(const Image&) = delete;
		Image(Image&&) = delete;

		~Image() noexcept = default;

		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = delete;

		VkImage image;
		VkImageView imageView;
		VkDeviceMemory memory;
	};

} // namespace lux::rhi

#endif // IMAGE_H_INCLUDED