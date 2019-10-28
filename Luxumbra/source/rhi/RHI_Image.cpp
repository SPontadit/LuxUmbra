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
		imageCI.mipLevels = luxImageCI.mipmapCount;
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
		imageViewCI.subresourceRange.levelCount = luxImageCI.mipmapCount;
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

	void RHI::GenerateCubemapFromHDR(const Image& HDRSource, Image& cubemap) noexcept
	{
		rhi::CubeMapCreateInfo cubemapCI = {};
		cubemapCI.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		cubemapCI.size = CUBEMAP_TEXTURE_SIZE;
		cubemapCI.binaryVertexFilePath = "data/shaders/generateCubeMap/generateCubeMap.vert.spv";
		cubemapCI.binaryFragmentFilePath = "data/shaders/generateCubeMap/generateCubeMap.frag.spv";
		cubemapCI.sampler = forward.sampler;
		cubemapCI.mipmapCount = TO_UINT32_T(floor(log2(CUBEMAP_TEXTURE_SIZE))) + 1;

		GenerateCubemap(cubemapCI, HDRSource, cubemap);
	}

	void RHI::GenerateIrradianceFromCubemap(const Image& cubemapSource, Image& irradiance) noexcept
	{
		//rhi::CubeMapCreateInfo irradianceCI = {};
		//irradianceCI.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		//irradianceCI.size = IRRADIANCE_TEXTURE_SIZE;
		//irradianceCI.binaryVertexFilePath = "data/shaders/generateIrradianceMap/generateIrradianceMap.vert.spv";
		//irradianceCI.binaryFragmentFilePath = "data/shaders/generateIrradianceMap/generateIrradianceMap.frag.spv";
		//irradianceCI.sampler = forward.cubemapSampler;
		//irradianceCI.mipmapCount = TO_UINT32_T(floor(log2(IRRADIANCE_TEXTURE_SIZE))) + 1;

		//GenerateCubemap(irradianceCI, cubemapSource, irradiance);


		VkDescriptorSetLayout generateIrradianceMapSetLayout = computeDescriptorSetLayout;
		VkDescriptorSetAllocateInfo generateIrradianceDescriptorSetAI = {};
		generateIrradianceDescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		generateIrradianceDescriptorSetAI.descriptorPool = computeDescriptorPool;
		generateIrradianceDescriptorSetAI.descriptorSetCount = 1;
		generateIrradianceDescriptorSetAI.pSetLayouts = &generateIrradianceMapSetLayout;

		CHECK_VK(vkAllocateDescriptorSets(device, &generateIrradianceDescriptorSetAI, &generateIrradianceDescriptorSet));


		VkDescriptorImageInfo computeEnvMapMapDescriptorImageInfo = {};
		computeEnvMapMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		computeEnvMapMapDescriptorImageInfo.imageView = cubemapSource.imageView;
		computeEnvMapMapDescriptorImageInfo.sampler = forward.cubemapSampler;

		VkWriteDescriptorSet writeComputeEnvMapDescriptorSet = {};
		writeComputeEnvMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeComputeEnvMapDescriptorSet.descriptorCount = 1;
		writeComputeEnvMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeComputeEnvMapDescriptorSet.dstBinding = 0;
		writeComputeEnvMapDescriptorSet.dstArrayElement = 0;
		writeComputeEnvMapDescriptorSet.pImageInfo = &computeEnvMapMapDescriptorImageInfo;
		writeComputeEnvMapDescriptorSet.dstSet = generateIrradianceDescriptorSet;


		VkDescriptorImageInfo computeIrradianceMapDescriptorImageInfo = {};
		computeIrradianceMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		computeIrradianceMapDescriptorImageInfo.imageView = irradiance.imageView;

		VkWriteDescriptorSet writeComputeIrradianceMapDescriptorSet = {};
		writeComputeIrradianceMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeComputeIrradianceMapDescriptorSet.descriptorCount = 1;
		writeComputeIrradianceMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		writeComputeIrradianceMapDescriptorSet.dstBinding = 1;
		writeComputeIrradianceMapDescriptorSet.dstArrayElement = 0;
		writeComputeIrradianceMapDescriptorSet.pImageInfo = &computeIrradianceMapDescriptorImageInfo;
		writeComputeIrradianceMapDescriptorSet.dstSet = generateIrradianceDescriptorSet;

		std::array<VkWriteDescriptorSet, 2> writeDescriptorSets =
		{
			writeComputeEnvMapDescriptorSet,
			writeComputeIrradianceMapDescriptorSet
		};

		vkUpdateDescriptorSets(device, TO_UINT32_T(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		GenerateIrradianceParameters parameters;
		parameters.deltaPhi = (2.0f * PI) / 180.0f;
		parameters.deltaTheta = (0.5f * PI) / TO_FLOAT(IRRADIANCE_TEXTURE_SIZE);
		parameters.cubemapSize = glm::vec2(CUBEMAP_TEXTURE_SIZE);

		VkCommandBufferBeginInfo commandBufferBI = {};
		commandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CHECK_VK(vkBeginCommandBuffer(computeCommandBuffer, &commandBufferBI));
		
		//CommandTransitionImageLayout(computeCommandBuffer, cubemapSource.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, 6, TO_UINT32_T(floor(log2(CUBEMAP_TEXTURE_SIZE))) + 1);
		CommandTransitionImageLayout(computeCommandBuffer, irradiance.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 6, TO_UINT32_T(floor(log2(IRRADIANCE_TEXTURE_SIZE))) + 1);

		vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

		vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &generateIrradianceDescriptorSet, 0, nullptr);

		vkCmdPushConstants(computeCommandBuffer, computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GenerateIrradianceParameters), &parameters);

		vkCmdDispatch(computeCommandBuffer, CUBEMAP_TEXTURE_SIZE / 32, CUBEMAP_TEXTURE_SIZE / 32, 6);

		CommandTransitionImageLayout(computeCommandBuffer, irradiance.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6, TO_UINT32_T(floor(log2(IRRADIANCE_TEXTURE_SIZE))) + 1);
		//CommandTransitionImageLayout(computeCommandBuffer, cubemapSource.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6, TO_UINT32_T(floor(log2(CUBEMAP_TEXTURE_SIZE))) + 1);

		CHECK_VK(vkEndCommandBuffer(computeCommandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &computeCommandBuffer;

		CHECK_VK(vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE));
		CHECK_VK(vkQueueWaitIdle(computeQueue));


		// Update Descriptor Set
		VkDescriptorImageInfo irradianceMapDescriptorImageInfo = {};
		irradianceMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		irradianceMapDescriptorImageInfo.sampler = forward.irradianceSampler;
		irradianceMapDescriptorImageInfo.imageView = irradiance.imageView;

		VkWriteDescriptorSet writeIrradianceMapDescriptorSet = {};
		writeIrradianceMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeIrradianceMapDescriptorSet.descriptorCount = 1;
		writeIrradianceMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeIrradianceMapDescriptorSet.dstBinding = 2;
		writeIrradianceMapDescriptorSet.dstArrayElement = 0;
		writeIrradianceMapDescriptorSet.pImageInfo = &irradianceMapDescriptorImageInfo;

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			writeIrradianceMapDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			vkUpdateDescriptorSets(device, 1, &writeIrradianceMapDescriptorSet, 0, nullptr);
		}
	}

	void RHI::GeneratePrefilteredFromCubemap(const Image& cubemapSource, Image& prefiltered) noexcept
	{
		rhi::CubeMapCreateInfo prefilteredCI = {};
		prefilteredCI.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		prefilteredCI.size = PREFILTERED_TEXTURE_SIZE;
		prefilteredCI.binaryVertexFilePath = "data/shaders/generatePrefilteredMap/generatePrefilteredMap.vert.spv";
		prefilteredCI.binaryFragmentFilePath = "data/shaders/generatePrefilteredMap/generatePrefilteredMap.frag.spv";
		prefilteredCI.sampler = forward.cubemapSampler;
		prefilteredCI.mipmapCount = TO_UINT32_T(floor(log2(PREFILTERED_TEXTURE_SIZE))) + 1;

		GenerateCubemap(prefilteredCI, cubemapSource, prefiltered);

		// Update Descriptor Set
		VkDescriptorImageInfo prefilteredMapDescriptorImageInfo = {};
		prefilteredMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		prefilteredMapDescriptorImageInfo.sampler = forward.prefilteredSampler;
		prefilteredMapDescriptorImageInfo.imageView = prefiltered.imageView;

		VkWriteDescriptorSet writePrefilteredMapDescriptorSet = {};
		writePrefilteredMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writePrefilteredMapDescriptorSet.descriptorCount = 1;
		writePrefilteredMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writePrefilteredMapDescriptorSet.dstBinding = 3;
		writePrefilteredMapDescriptorSet.dstArrayElement = 0;
		writePrefilteredMapDescriptorSet.pImageInfo = &prefilteredMapDescriptorImageInfo;

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			writePrefilteredMapDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			vkUpdateDescriptorSets(device, 1, &writePrefilteredMapDescriptorSet, 0, nullptr);
		}
	}

	void RHI::GenerateCubemap(const CubeMapCreateInfo& luxCubemapCI, const Image& source, Image& image) noexcept
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
		attachmentDescription.format = luxCubemapCI.format;
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
		imageCI.format = luxCubemapCI.format;
		imageCI.width = luxCubemapCI.size;
		imageCI.height = luxCubemapCI.size;
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
		framebufferCI.width = luxCubemapCI.size;
		framebufferCI.height = luxCubemapCI.size;
		framebufferCI.layers = 1;

		CHECK_VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &offscreen.framebuffer));

		CommandTransitionImageLayout(offscreen.image.image, luxCubemapCI.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VkDescriptorPoolSize descriptorPoolSize = {};
		descriptorPoolSize.descriptorCount = 1;
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = 1;
		descriptorPoolCI.pPoolSizes = &descriptorPoolSize;
		descriptorPoolCI.maxSets = 1;

		CHECK_VK(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &offscreen.descriptorPool));

		// TODO: Find solution for dimension parameter
		struct PushConstant
		{
			glm::mat4 mvp;
			float deltaPhi = (2.0f * PI) / 180.0f;
			float deltaTheta = (0.5f * PI) / TO_FLOAT(IRRADIANCE_TEXTURE_SIZE); //dimension
			float roughness;
			uint32_t samplersCount = 32u;
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
		graphicsPipelineCI.binaryVertexFilePath = luxCubemapCI.binaryVertexFilePath;
		graphicsPipelineCI.binaryFragmentFilePath = luxCubemapCI.binaryFragmentFilePath;
		graphicsPipelineCI.vertexLayout = lux::VertexLayout::VERTEX_BASIC_LAYOUT;
		graphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		graphicsPipelineCI.viewportWidth = TO_FLOAT(luxCubemapCI.size); // TODO: Change if issue -> swapchain extent
		graphicsPipelineCI.viewportHeight = TO_FLOAT(luxCubemapCI.size);
		graphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		graphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		graphicsPipelineCI.disableMSAA = VK_TRUE;
		graphicsPipelineCI.enableDepthTest = VK_FALSE;
		graphicsPipelineCI.enableDepthWrite = VK_FALSE;
		graphicsPipelineCI.enableDepthBias = VK_FALSE;
		graphicsPipelineCI.depthBiasConstantFactor = 0.f;
		graphicsPipelineCI.depthBiasSlopeFactor = 0.f;
		graphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		graphicsPipelineCI.viewDescriptorSetLayoutBindings = { descriptorSetLayoutBinding };
		graphicsPipelineCI.pushConstants = { pushConstantRange };
		graphicsPipelineCI.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		CreateGraphicsPipeline(graphicsPipelineCI, offscreen.pipeline);

		VkDescriptorSetAllocateInfo descriptorSetAI = {};
		descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAI.descriptorPool = offscreen.descriptorPool;
		descriptorSetAI.descriptorSetCount = 1;
		descriptorSetAI.pSetLayouts = &offscreen.pipeline.viewDescriptorSetLayout;

		CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAI, &offscreen.descriptorSet));


		VkDescriptorImageInfo descriptorImageInfo = {};
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImageInfo.sampler = luxCubemapCI.sampler;
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
		renderPassBI.renderArea.extent.width = luxCubemapCI.size;
		renderPassBI.renderArea.extent.height = luxCubemapCI.size;
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

		CommandTransitionImageLayout(commandBuffer, image.image, luxCubemapCI.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, luxCubemapCI.mipmapCount);

		VkDeviceSize offset[] = { 0 };

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.width = TO_FLOAT(luxCubemapCI.size);
		viewport.height = TO_FLOAT(luxCubemapCI.size);

		VkRect2D scissor = {};
		scissor.extent = { luxCubemapCI.size, luxCubemapCI.size };
		scissor.offset = { 0, 0 };

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		for (uint32_t mipmap = 0; mipmap < luxCubemapCI.mipmapCount; ++mipmap)
		{
			pushConstant.roughness = TO_FLOAT(mipmap) / TO_FLOAT(luxCubemapCI.mipmapCount - 1);


			for (uint32_t face = 0; face < 6; face++)
			{
				viewport.width = TO_FLOAT(luxCubemapCI.size * std::pow(0.5f, mipmap));
				viewport.height = TO_FLOAT(luxCubemapCI.size * std::pow(0.5f, mipmap));
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

				vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

				pushConstant.mvp = glm::perspective(PI / 2.0f, 1.0f, 0.001f, 1000.0f) * matrices[face];
				vkCmdPushConstants(commandBuffer, offscreen.pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pushConstant);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipeline);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipelineLayout, 0, 1, &offscreen.descriptorSet, 0, nullptr);

				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->vertexBuffer.buffer, offset);
				vkCmdBindIndexBuffer(commandBuffer, cube->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(commandBuffer, cube->indexCount, 1, 0, 0, 0);

				vkCmdEndRenderPass(commandBuffer);

				CommandTransitionImageLayout(commandBuffer, offscreen.image.image, luxCubemapCI.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

				VkImageCopy imageCopy = {};
				imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageCopy.srcSubresource.baseArrayLayer = 0;
				imageCopy.srcSubresource.mipLevel = 0;
				imageCopy.srcSubresource.layerCount = 1;
				imageCopy.srcOffset = { 0, 0, 0 };

				imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageCopy.dstSubresource.baseArrayLayer = face;
				imageCopy.dstSubresource.mipLevel = mipmap;
				imageCopy.dstSubresource.layerCount = 1;
				imageCopy.dstOffset = { 0,0,0 };

				imageCopy.extent.width = TO_UINT32_T(viewport.width);
				imageCopy.extent.height = TO_UINT32_T(viewport.height);
				imageCopy.extent.depth = 1;

				vkCmdCopyImage(commandBuffer, offscreen.image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

				CommandTransitionImageLayout(commandBuffer, offscreen.image.image, luxCubemapCI.format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			}

		}

		CommandTransitionImageLayout(commandBuffer, image.image, luxCubemapCI.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6, luxCubemapCI.mipmapCount);

		EndSingleTimeCommandBuffer(commandBuffer);

		DestroyGraphicsPipeline(offscreen.pipeline);
		vkDestroyDescriptorPool(device, offscreen.descriptorPool, nullptr);
		vkDestroyFramebuffer(device, offscreen.framebuffer, nullptr);
		DestroyImage(offscreen.image);
		vkDestroyRenderPass(device, offscreen.renderPass, nullptr);
	}

	void RHI::GenerateBRDFLut(VkFormat format, uint32_t size, Image& BRDFLut) noexcept
	{
		ImageCreateInfo imageCI = {};
		imageCI.arrayLayers = 1;
		imageCI.format = format;
		imageCI.width = size;
		imageCI.height = size;
		imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
		imageCI.subresourceRangeLayerCount = 1;
		imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		CreateImage(imageCI, BRDFLut);


		struct OffscreenResource
		{
			VkRenderPass renderPass;
			VkFramebuffer framebuffer;
			GraphicsPipeline pipeline;
		} offscreen;

		// Attachment
		VkAttachmentDescription attachmentDescription = {};
		attachmentDescription.format = format;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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

		VkFramebufferCreateInfo framebufferCI = {};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = offscreen.renderPass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &BRDFLut.imageView;
		framebufferCI.width = size;
		framebufferCI.height = size;
		framebufferCI.layers = 1;

		CHECK_VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &offscreen.framebuffer));

		struct PushConstant
		{
			int sampleCount = 1024;
		} pushConstant;

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.size = sizeof(PushConstant);
		pushConstantRange.stageFlags =  VK_SHADER_STAGE_FRAGMENT_BIT;

		GraphicsPipelineCreateInfo graphicsPipelineCI = {};
		graphicsPipelineCI.renderPass = offscreen.renderPass;
		graphicsPipelineCI.subpassIndex = 0;
		graphicsPipelineCI.binaryVertexFilePath = "data/shaders/generateBRDFLut/generateBRDFLut.vert.spv";
		graphicsPipelineCI.binaryFragmentFilePath = "data/shaders/generateBRDFLut/generateBRDFLut.frag.spv";
		graphicsPipelineCI.vertexLayout = lux::VertexLayout::NO_VERTEX_LAYOUT;
		graphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		graphicsPipelineCI.viewportWidth = TO_FLOAT(size); // TODO: Change if issue -> swapchain extent
		graphicsPipelineCI.viewportHeight = TO_FLOAT(size);
		graphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		graphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		graphicsPipelineCI.disableMSAA = true;
		graphicsPipelineCI.enableDepthTest = false;
		graphicsPipelineCI.enableDepthWrite = false;
		graphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		graphicsPipelineCI.pushConstants = { pushConstantRange };

		CreateGraphicsPipeline(graphicsPipelineCI, offscreen.pipeline);

		VkClearValue clearValue;
		clearValue.color = { 0.0f, 0.0f, 0.0f };

		VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

		VkRenderPassBeginInfo renderPassBI = {};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.framebuffer = offscreen.framebuffer;
		renderPassBI.clearValueCount = 1;
		renderPassBI.pClearValues = &clearValue;
		renderPassBI.renderArea.extent.width = size;
		renderPassBI.renderArea.extent.height = size;
		renderPassBI.renderPass = offscreen.renderPass;

		vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipeline);

		pushConstant.sampleCount = 1024;

		vkCmdPushConstants(commandBuffer, offscreen.pipeline.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pushConstant);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		EndSingleTimeCommandBuffer(commandBuffer);

		DestroyGraphicsPipeline(offscreen.pipeline);
		vkDestroyFramebuffer(device, offscreen.framebuffer, nullptr);
		vkDestroyRenderPass(device, offscreen.renderPass, nullptr);

		// Update Descriptor Set
		VkDescriptorImageInfo BRDFLutDescriptorImageInfo = {};
		BRDFLutDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		BRDFLutDescriptorImageInfo.sampler = forward.prefilteredSampler;
		BRDFLutDescriptorImageInfo.imageView = BRDFLut.imageView;

		VkWriteDescriptorSet writeBRDFLutDescriptorSet = {};
		writeBRDFLutDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeBRDFLutDescriptorSet.descriptorCount = 1;
		writeBRDFLutDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeBRDFLutDescriptorSet.dstBinding = 4;
		writeBRDFLutDescriptorSet.dstArrayElement = 0;
		writeBRDFLutDescriptorSet.pImageInfo = &BRDFLutDescriptorImageInfo;

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			writeBRDFLutDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			vkUpdateDescriptorSets(device, 1, &writeBRDFLutDescriptorSet, 0, nullptr);
		}
	}

	void RHI::DestroyImage(Image& image) noexcept
	{
		vkDestroyImageView(device, image.imageView, nullptr);
		vkDestroyImage(device, image.image, nullptr);
		vkFreeMemory(device, image.memory, nullptr);
	}


} // namespace lux::rhi