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

	void RHI::GenerateCubemap(const ImageCreateInfo& luxImageCI, Image& source, Image& image) noexcept
	{
		struct OffscreenResource
		{
			VkRenderPass renderPass;
			Image image;
			VkFramebuffer framebuffer;
			VkDescriptorPool descriptorPool;
			VkDescriptorSet descriptorSet;
			GraphicsPipeline pipeline;
		} offscreen;

		uint32_t dimension = 1024;

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

		ImageCreateInfo imageCI = {};
		imageCI.format = luxImageCI.format;
		imageCI.width = dimension;
		imageCI.height = dimension;
		imageCI.arrayLayers = 1;
		imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCI.subresourceRangeLayerCount = 1;
		imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(imageCI, offscreen.image);

		VkFramebufferCreateInfo framebufferCI = {};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = offscreen.renderPass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &offscreen.image.imageView;
		framebufferCI.width = dimension;
		framebufferCI.height = dimension;
		framebufferCI.layers = 1;

		CHECK_VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &offscreen.framebuffer));

		CommandTransitionImageLayout(offscreen.image.image, luxImageCI.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VkDescriptorPoolSize descriptorPoolSize = {};
		descriptorPoolSize.descriptorCount = 1;
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = 1;
		descriptorPoolCI.pPoolSizes = &descriptorPoolSize;
		descriptorPoolCI.maxSets = 1;

		CHECK_VK(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &offscreen.descriptorPool));

		struct PushConstant
		{
			glm::mat4 mvp;
		} pushConstant;

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.size = sizeof(PushConstant);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
		descriptorSetLayoutBinding.binding = 0;
		descriptorSetLayoutBinding.descriptorCount = 1;
		descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		
		GraphicsPipelineCreateInfo graphicsPipelineCI = {};
		graphicsPipelineCI.renderPass = offscreen.renderPass;
		graphicsPipelineCI.subpassIndex = 0;
		graphicsPipelineCI.binaryVertexFilePath = "data/shaders/generateCubeMap/generateCubeMap.vert.spv";
		graphicsPipelineCI.binaryFragmentFilePath = "data/shaders/generateCubeMap/generateCubeMap.frag.spv";
		graphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		graphicsPipelineCI.viewportWidth = TO_FLOAT(dimension); // TODO: Change if issue -> swapchain extent
		graphicsPipelineCI.viewportHeight = TO_FLOAT(dimension);
		graphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		graphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		graphicsPipelineCI.enableDepthTest = false;
		graphicsPipelineCI.enableDepthWrite = false;
		graphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		graphicsPipelineCI.viewDescriptorSetLayoutBindings = { descriptorSetLayoutBinding };
		graphicsPipelineCI.pushConstants = { pushConstantRange };

		CreateGraphicsPipeline(graphicsPipelineCI, offscreen.pipeline);

		VkDescriptorSetAllocateInfo descriptorSetAI = {};
		descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAI.descriptorPool = offscreen.descriptorPool;
		descriptorSetAI.descriptorSetCount = 1;
		descriptorSetAI.pSetLayouts = &offscreen.pipeline.viewDescriptorSetLayout;

		CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAI, &offscreen.descriptorSet));


		VkDescriptorImageInfo descriptorImageInfo = {};
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImageInfo.sampler = forward.sampler;
		descriptorImageInfo.imageView = source.imageView;

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.dstSet = offscreen.descriptorSet;
		writeDescriptorSet.pImageInfo = &descriptorImageInfo;

		vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);



		// Draw offscreen
		VkClearValue clearValue;
		clearValue.color = { 0.0f, 0.0f, 0.0f };

		VkRenderPassBeginInfo renderPassBI = {};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.framebuffer = offscreen.framebuffer;
		renderPassBI.clearValueCount = 1;
		renderPassBI.pClearValues = &clearValue;
		renderPassBI.renderArea.extent.width = dimension;
		renderPassBI.renderArea.extent.height = dimension;
		renderPassBI.renderPass = offscreen.renderPass;


		std::array<glm::mat4, 6> matrices = 
		{
			// POSITIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

		CommandTransitionImageLayout(commandBuffer, image.image, luxImageCI.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);

		VkDeviceSize offset[] = { 0 };
		for (uint32_t i = 0; i < 6; i++)
		{
			vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

			pushConstant.mvp = glm::perspective(PI / 2.0f, 1.0f, 0.001f, 1000.0f) * matrices[i];
			vkCmdPushConstants(commandBuffer, offscreen.pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pushConstant);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipeline);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipelineLayout, 0, 1, &offscreen.descriptorSet, 0, nullptr);
		
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->vertexBuffer.buffer, offset);
			vkCmdBindIndexBuffer(commandBuffer, cube->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, cube->indexCount, 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffer);

			CommandTransitionImageLayout(commandBuffer, offscreen.image.image, luxImageCI.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			VkImageCopy imageCopy = {};
			imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopy.srcSubresource.baseArrayLayer = 0;
			imageCopy.srcSubresource.mipLevel = 0;
			imageCopy.srcSubresource.layerCount = 1;
			imageCopy.srcOffset = { 0, 0, 0 };

			imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopy.dstSubresource.baseArrayLayer = i;
			imageCopy.dstSubresource.mipLevel = 0;
			imageCopy.dstSubresource.layerCount = 1;
			imageCopy.dstOffset = { 0,0,0 };

			imageCopy.extent.width = dimension;
			imageCopy.extent.height = dimension;
			imageCopy.extent.depth = 1;
		
			vkCmdCopyImage(commandBuffer, offscreen.image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
		
			CommandTransitionImageLayout(commandBuffer, offscreen.image.image, luxImageCI.format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		CommandTransitionImageLayout(commandBuffer, image.image, luxImageCI.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);

		EndSingleTimeCommandBuffer(commandBuffer);

		DestroyGraphicsPipeline(offscreen.pipeline);
		vkDestroyDescriptorPool(device, offscreen.descriptorPool, nullptr);
		vkDestroyFramebuffer(device, offscreen.framebuffer, nullptr);
		DestroyImage(offscreen.image);
		vkDestroyRenderPass(device, offscreen.renderPass, nullptr);
	}

	void RHI::GenerateIrradianceMap(const ImageCreateInfo& luxImageCI, Image& source, Image& image) noexcept
	{
		struct OffscreenResource
		{
			VkRenderPass renderPass;
			Image image;
			VkFramebuffer framebuffer;
			VkDescriptorPool descriptorPool;
			VkDescriptorSet descriptorSet;
			GraphicsPipeline pipeline;
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

		std::array<VkSubpassDependency, 2> subpassDependencies{ subpassDependency_0, subpassDependency_1 };


		VkRenderPassCreateInfo renderPassCI = {};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &attachmentDescription;
		renderPassCI.dependencyCount = TO_UINT32_T(subpassDependencies.size());
		renderPassCI.pDependencies = subpassDependencies.data();
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;

		CHECK_VK(vkCreateRenderPass(device, &renderPassCI, nullptr, &offscreen.renderPass));

		ImageCreateInfo imageCI = {};
		imageCI.format = luxImageCI.format;
		imageCI.width = luxImageCI.width;
		imageCI.height = luxImageCI.width;
		imageCI.arrayLayers = 1;
		imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCI.subresourceRangeLayerCount = 1;
		imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(imageCI, offscreen.image);

		VkFramebufferCreateInfo framebufferCI = {};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = offscreen.renderPass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &offscreen.image.imageView;
		framebufferCI.width = luxImageCI.width;
		framebufferCI.height = luxImageCI.width;
		framebufferCI.layers = 1;

		CHECK_VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &offscreen.framebuffer));

		CommandTransitionImageLayout(offscreen.image.image, luxImageCI.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VkDescriptorPoolSize descriptorPoolSize = {};
		descriptorPoolSize.descriptorCount = 1;
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = 1;
		descriptorPoolCI.pPoolSizes = &descriptorPoolSize;
		descriptorPoolCI.maxSets = 1;

		CHECK_VK(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &offscreen.descriptorPool));

		struct PushConstant
		{
			glm::mat4 mvp;
			float deltaPhi = (2.0f * PI) / 180.0f;
			float deltaTheta = (0.5f * PI) / 124.0f; //dimensiom
		} pushConstant;

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.size = sizeof(PushConstant);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
		descriptorSetLayoutBinding.binding = 0;
		descriptorSetLayoutBinding.descriptorCount = 1;
		descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		GraphicsPipelineCreateInfo graphicsPipelineCI = {};
		graphicsPipelineCI.renderPass = offscreen.renderPass;
		graphicsPipelineCI.subpassIndex = 0;
		graphicsPipelineCI.binaryVertexFilePath = "data/shaders/generateIrradianceMap/generateIrradianceMap.vert.spv";
		graphicsPipelineCI.binaryFragmentFilePath = "data/shaders/generateIrradianceMap/generateIrradianceMap.frag.spv";
		graphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		graphicsPipelineCI.viewportWidth = TO_FLOAT(luxImageCI.width); // TODO: Change if issue -> swapchain extent
		graphicsPipelineCI.viewportHeight = TO_FLOAT(luxImageCI.width);
		graphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		graphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		graphicsPipelineCI.enableDepthTest = false;
		graphicsPipelineCI.enableDepthWrite = false;
		graphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		graphicsPipelineCI.viewDescriptorSetLayoutBindings = { descriptorSetLayoutBinding };
		graphicsPipelineCI.pushConstants = { pushConstantRange };

		CreateGraphicsPipeline(graphicsPipelineCI, offscreen.pipeline);

		VkDescriptorSetAllocateInfo descriptorSetAI = {};
		descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAI.descriptorPool = offscreen.descriptorPool;
		descriptorSetAI.descriptorSetCount = 1;
		descriptorSetAI.pSetLayouts = &offscreen.pipeline.viewDescriptorSetLayout;

		CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAI, &offscreen.descriptorSet));


		VkDescriptorImageInfo descriptorImageInfo = {};
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImageInfo.sampler = forward.sampler;
		descriptorImageInfo.imageView = source.imageView;

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.dstSet = offscreen.descriptorSet;
		writeDescriptorSet.pImageInfo = &descriptorImageInfo;

		vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);



		// Draw offscreen
		VkClearValue clearValue;
		clearValue.color = { 0.0f, 0.0f, 0.0f };

		VkRenderPassBeginInfo renderPassBI = {};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.framebuffer = offscreen.framebuffer;
		renderPassBI.clearValueCount = 1;
		renderPassBI.pClearValues = &clearValue;
		renderPassBI.renderArea.extent.width = luxImageCI.width;
		renderPassBI.renderArea.extent.height = luxImageCI.width;
		renderPassBI.renderPass = offscreen.renderPass;


		std::array<glm::mat4, 6> matrices =
		{
			// POSITIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();
		
		CommandTransitionImageLayout(commandBuffer, image.image, luxImageCI.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);

		VkDeviceSize offset[] = { 0 };
		for (uint32_t i = 0; i < 6; i++)
		{
			vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

			pushConstant.mvp = glm::perspective(PI / 2.0f, 1.0f, 0.001f, 1000.0f) * matrices[i];
			vkCmdPushConstants(commandBuffer, offscreen.pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pushConstant);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipeline);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipelineLayout, 0, 1, &offscreen.descriptorSet, 0, nullptr);

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->vertexBuffer.buffer, offset);
			vkCmdBindIndexBuffer(commandBuffer, cube->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, cube->indexCount, 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffer);

			CommandTransitionImageLayout(commandBuffer, offscreen.image.image, luxImageCI.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			VkImageCopy imageCopy = {};
			imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopy.srcSubresource.baseArrayLayer = 0;
			imageCopy.srcSubresource.mipLevel = 0;
			imageCopy.srcSubresource.layerCount = 1;
			imageCopy.srcOffset = { 0, 0, 0 };

			imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopy.dstSubresource.baseArrayLayer = i;
			imageCopy.dstSubresource.mipLevel = 0;
			imageCopy.dstSubresource.layerCount = 1;
			imageCopy.dstOffset = { 0,0,0 };

			imageCopy.extent.width = luxImageCI.width;
			imageCopy.extent.height = luxImageCI.width;
			imageCopy.extent.depth = 1;

			vkCmdCopyImage(commandBuffer, offscreen.image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

			CommandTransitionImageLayout(commandBuffer, offscreen.image.image, luxImageCI.format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		CommandTransitionImageLayout(commandBuffer, image.image, luxImageCI.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);

		EndSingleTimeCommandBuffer(commandBuffer);

		DestroyGraphicsPipeline(offscreen.pipeline);
		vkDestroyDescriptorPool(device, offscreen.descriptorPool, nullptr);
		vkDestroyFramebuffer(device, offscreen.framebuffer, nullptr);
		DestroyImage(offscreen.image);
		vkDestroyRenderPass(device, offscreen.renderPass, nullptr);
	}


	void RHI::DestroyImage(Image& image) noexcept
	{
		vkDestroyImageView(device, image.imageView, nullptr);
		vkDestroyImage(device, image.image, nullptr);
		vkFreeMemory(device, image.memory, nullptr);
	}


} // namespace lux::rhi