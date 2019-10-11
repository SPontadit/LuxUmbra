#include "rhi\RHI.h"


namespace lux::rhi
{

	Buffer::Buffer() noexcept
		: buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE)
	{

	}

	void RHI::CreateBuffer(const BufferCreateInfo& luxBufferCI, Buffer& buffer) noexcept
	{
		buffer.size = luxBufferCI.size;

		VkBufferCreateInfo bufferCI = {};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = buffer.size;
		bufferCI.usage = luxBufferCI.usageFlags;
		bufferCI.sharingMode = luxBufferCI.sharingMode;

		CHECK_VK(vkCreateBuffer(device, &bufferCI, nullptr, &buffer.buffer));
	
		
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, buffer.buffer, &memoryRequirements);
		uint32_t memoryIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		ASSERT(memoryIndex != UINT32_MAX);

		VkMemoryAllocateInfo memoryAI = {};
		memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAI.allocationSize = memoryRequirements.size;
		memoryAI.memoryTypeIndex = memoryIndex;

		CHECK_VK(vkAllocateMemory(device, &memoryAI, nullptr, &buffer.memory));
		CHECK_VK(vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0));

		UpdateBuffer(buffer, luxBufferCI.data);
	}

	void RHI::UpdateBuffer(Buffer& buffer, void* newData) noexcept
	{
		void* bufferData;
		CHECK_VK(vkMapMemory(device, buffer.memory, 0, buffer.size, 0, &bufferData));
		memcpy(bufferData, newData, TO_SIZE_T(buffer.size));
		vkUnmapMemory(device, buffer.memory);
	}

	void RHI::DestroyBuffer(Buffer& buffer) noexcept
	{
		vkDestroyBuffer(device, buffer.buffer, nullptr);
		vkFreeMemory(device, buffer.memory, nullptr);
	}


} // namespace lux::rhi