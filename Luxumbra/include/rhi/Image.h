#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include "Luxumbra.h"

#include "rhi\LuxVkImpl.h"

namespace lux::rhi
{
	struct ImageCreateInfo
	{
		VkFormat format;
		uint32_t width;
		uint32_t height;
		VkImageUsageFlagBits usage;
		VkImageAspectFlagBits subresourceRangeAspectMask;
	};


	struct Image
	{
		Image() noexcept;
		Image(const Image&) noexcept;
		Image(Image&&) noexcept;

		~Image() noexcept = default;

		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = delete;

		VkImage image;
		VkImageView imageView;
		VkDeviceMemory memory;
	};

} // namespace lux::rhi

#endif // TEXTURE_H_INCLUDED