#include "rhi\RHI.h"

#include <array>

#include "glm\gtc\matrix_transform.hpp"


namespace lux::rhi
{
	ForwardRenderer::ForwardRenderer() noexcept
		: renderPass(VK_NULL_HANDLE), frameBuffers(0), blitGraphicsPipeline(),
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
		rtDepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

	void RHI::InitForwardDescriptorPool() noexcept
	{
		VkDescriptorPoolSize blitDescriptorPoolSize = {};
		blitDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		blitDescriptorPoolSize.descriptorCount = swapchainImageCount;

		std::array<VkDescriptorPoolSize, 1> descriptorPoolSizes = { blitDescriptorPoolSize };

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = TO_UINT32_T(descriptorPoolSizes.size());
		descriptorPoolCI.pPoolSizes = descriptorPoolSizes.data();
		descriptorPoolCI.maxSets = swapchainImageCount;

		CHECK_VK(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &forward.descriptorPool));
	}

	void RHI::InitForwardGraphicsPipelines() noexcept
	{
		// Blit Graphics Pipeline
		VkDescriptorSetLayoutBinding blitDescriptorSetLayoutBinding = {};
		blitDescriptorSetLayoutBinding.binding = 0;
		blitDescriptorSetLayoutBinding.descriptorCount = 1;
		blitDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		blitDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		GraphicsPipelineCreateInfo blitGraphicsPipelineCI = {};
		blitGraphicsPipelineCI.renderPass = forward.renderPass;
		blitGraphicsPipelineCI.subpassIndex = ForwardRenderer::FORWARD_SUBPASS_COPY;
		blitGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/blit/blit.vert.spv";
		blitGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/blit/blit.frag.spv";
		blitGraphicsPipelineCI.emptyVertexInput = true;
		blitGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		blitGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		blitGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		blitGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		blitGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		blitGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		blitGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		blitGraphicsPipelineCI.descriptorSetLayoutBindings = { blitDescriptorSetLayoutBinding };

		CreateGraphicsPipeline(blitGraphicsPipelineCI, forward.blitGraphicsPipeline);


		// Render Target Graphics Pipeline
		VkDescriptorSetLayoutBinding rtUboDescriptorSetLayoutBinding = {};
		rtUboDescriptorSetLayoutBinding.binding = 0;
		rtUboDescriptorSetLayoutBinding.descriptorCount = 1;
		rtUboDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		rtUboDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding rtSamplerDescriptorSetLayoutBinding = {};
		rtSamplerDescriptorSetLayoutBinding.binding = 1;
		rtSamplerDescriptorSetLayoutBinding.descriptorCount = 1;
		rtSamplerDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		rtSamplerDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPushConstantRange rtPushConstantRange = {};
		rtPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		rtPushConstantRange.size = sizeof(RtConstants);
		rtPushConstantRange.offset = 0;

		GraphicsPipelineCreateInfo rtGraphicsPipelineCI = {};
		rtGraphicsPipelineCI.renderPass = forward.renderPass;
		rtGraphicsPipelineCI.subpassIndex = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		rtGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/triangle/triangle.vert.spv";
		rtGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/triangle/triangle.frag.spv";
		rtGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		rtGraphicsPipelineCI.emptyVertexInput = true;
		rtGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		rtGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		rtGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		rtGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rtGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		rtGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		//rtGraphicsPipelineCI.pushConstants = { rtPushConstantRange };

		CreateGraphicsPipeline(rtGraphicsPipelineCI, forward.rtGraphicsPipeline);

		float aspectRatio = TO_FLOAT(swapchainExtent.width) / TO_FLOAT(swapchainExtent.height);
		forward.rtConstants.projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
		forward.rtConstants.projection[1][1] *= -1.0f;

		glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -5.0f);
		forward.rtConstants.view = glm::translate(glm::mat4(1.0f), cameraPosition);
	}

	void RHI::InitForwardDescriptorSets() noexcept
	{
		// Allocate Blit Descriptor Sets
		std::vector<VkDescriptorSetLayout> descriptorSetLayout(swapchainImageCount, forward.blitGraphicsPipeline.descriptorSetLayout);
		VkDescriptorSetAllocateInfo descriptorSetAI = {};
		descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAI.descriptorPool = forward.descriptorPool;
		descriptorSetAI.descriptorSetCount = swapchainImageCount;
		descriptorSetAI.pSetLayouts = descriptorSetLayout.data();

		forward.blitDescriptorSets.resize(TO_SIZE_T(swapchainImageCount));
		CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAI, forward.blitDescriptorSets.data()));
	
	
		// Update Blit Descriptor Sets
		VkDescriptorImageInfo blitDescriptorImageInfo = {};
		blitDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		blitDescriptorImageInfo.sampler = VK_NULL_HANDLE;

		VkWriteDescriptorSet blitWriteDescriptorSet = {};
		blitWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		blitWriteDescriptorSet.descriptorCount = 1;
		blitWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		blitWriteDescriptorSet.dstBinding = 0;
		blitWriteDescriptorSet.pImageInfo = &blitDescriptorImageInfo;

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			blitDescriptorImageInfo.imageView = forward.rtColorAttachmentImageViews[i];
			blitWriteDescriptorSet.dstSet = forward.blitDescriptorSets[i];
		
			vkUpdateDescriptorSets(device, 1, &blitWriteDescriptorSet, 0, nullptr);
		}
	}

	void RHI::RenderForward(const scene::CameraNode* camera, const std::vector<scene::MeshNode*> meshes) noexcept
	{
		forward.rtConstants.view = camera->GetViewTransform();
		forward.rtConstants.projection = camera->GetPerspectiveProjectionTransform();

		// Acquire next image
		VkFence* fence = &forward.fences[currentFrame];
		VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

		vkWaitForFences(device, 1, fence, false, UINT64_MAX);
		vkResetFences(device, 1, fence);
		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkSemaphore* acquireSemaphore = &forward.acquireSemaphores[currentFrame];
		VkSemaphore* presentSemaphore = &forward.presentSemaphores[currentFrame];

		uint32_t imageIndex;
		uint64_t timeout = UINT64_MAX;

		CHECK_VK(vkAcquireNextImageKHR(device, swapchain, timeout, *acquireSemaphore, VK_NULL_HANDLE, &imageIndex));


		// Begin Command Buffer
		VkCommandBufferBeginInfo commandBufferBI = {};
		commandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CHECK_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));


		VkClearColorValue clearColor{ 0.5f, 0.5703125f, 0.6171875f, 1.0F };

		std::array<VkClearValue, 3> clearValues = {};
		clearValues[ForwardRenderer::FORWARD_SWAPCHAIN_ATTACHMENT_BIND_POINT].color = clearColor;
		clearValues[ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT].color = clearColor;
		clearValues[ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT].depthStencil = { 1.0f, 0 };


		// Begin Render Pass
		VkRenderPassBeginInfo renderPassBI = {};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.renderPass = forward.renderPass;
		renderPassBI.framebuffer = forward.frameBuffers[imageIndex];
		renderPassBI.renderArea.extent = swapchainExtent;
		renderPassBI.clearValueCount = TO_UINT32_T(clearValues.size());
		renderPassBI.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);


		// Render Target Subpass
		vkCmdPushConstants(commandBuffer, forward.rtGraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RtConstants), &forward.rtConstants);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipeline);

		//VkDeviceSize offset[] = { 0 };
		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh->vertexBuffer.buffer, offset);
		//vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);	
		//vkCmdDrawIndexed(commandBuffer, mesh->indexCount, 1, 0, 0, 0);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		// Blit Subpass
		vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.blitGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.blitGraphicsPipeline.pipelineLayout, 0, 1, &forward.blitDescriptorSets[currentFrame], 0, NULL);

		vkCmdDraw(commandBuffer, 4, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		CHECK_VK(vkEndCommandBuffer(commandBuffer));

		// Submit Command Buffer
		VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.pWaitDstStageMask = &stageMask;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = acquireSemaphore;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = presentSemaphore;

		CHECK_VK(vkQueueSubmit(presentQueue, 1, &submitInfo, *fence));


		// Present
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = presentSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;

		CHECK_VK(vkQueuePresentKHR(graphicsQueue, &presentInfo));

		frameCount++;
		currentFrame = frameCount % swapchainImageCount;
	}


} // namespace lux::rhi