#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include "Luxumbra.h"

#include "rhi\LuxVkImpl.h"

namespace lux::rhi
{

	struct BufferCreateInfo
	{
		VkBufferUsageFlags usageFlags;
		VkDeviceSize size;
		VkSharingMode sharingMode;
		VkMemoryPropertyFlags memoryProperty;
		void* data;
	};

	struct Buffer
	{
		Buffer() noexcept;
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;

		~Buffer() noexcept = default;

		const Buffer& operator==(const Buffer&) = delete;
		const Buffer& operator==(Buffer&&) = delete;
		
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDeviceSize size;
	};

} // namespace lux::rhi

#endif // BUFFER_H_INCLUDED