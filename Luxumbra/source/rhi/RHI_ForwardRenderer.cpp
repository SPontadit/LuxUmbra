#include "rhi\RHI.h"

#include <array>
#include <map>

#include "glm\gtc\matrix_transform.hpp"


namespace lux::rhi
{
	using sortedMeshNodesIterator = std::map<std::string, std::vector<scene::MeshNode*>>::iterator;
	using sortedMeshNodesConstIterator = std::map<std::string, std::vector<scene::MeshNode*>>::const_iterator;
	
	ForwardRenderer::ForwardRenderer() noexcept
		: renderPass(VK_NULL_HANDLE), frameBuffers(0), descriptorPool(VK_NULL_HANDLE), blitGraphicsPipeline(), blitDescriptorSets(0),
		rtGraphicsPipeline(), rtViewDescriptorSets(0), rtModelDescriptorSets(0), rtColorAttachmentImages(0), rtColorAttachmentImageMemories(0), rtColorAttachmentImageViews(0),
		rtResolveColorAttachmentImage(VK_NULL_HANDLE), rtResolveColorAttachmentMemory(VK_NULL_HANDLE), rtResolveColorAttachmentImageView(VK_NULL_HANDLE),
		rtDepthAttachmentImage(VK_NULL_HANDLE), rtDepthAttachmentMemory(VK_NULL_HANDLE), rtDepthAttachmentImageView(VK_NULL_HANDLE),
		envMapGraphicsPipeline(), envMapViewDescriptorSets(0), modelConstant(), viewProjUniformBuffers(0), sampler(VK_NULL_HANDLE), cubemapSampler(VK_NULL_HANDLE)
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
		rtColorAttachment.samples = msaaSamples;
		rtColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription rtDepthAttachment = {};
		rtDepthAttachment.format = depthImageFormat;
		rtDepthAttachment.samples = msaaSamples;
		rtDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtDepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtDepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtDepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtDepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		VkAttachmentDescription rtResolveColorAttachment = {};
		rtResolveColorAttachment.format = swapchainImageFormat;
		rtResolveColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		rtResolveColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtResolveColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtResolveColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtResolveColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkAttachmentReference swapchainAttachmentRef = {};
		swapchainAttachmentRef.attachment = ForwardRenderer::FORWARD_SWAPCHAIN_ATTACHMENT_BIND_POINT;
		swapchainAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtColorAttachmentRef = {};
		rtColorAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT;
		rtColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtResolveColorAttachmentRef = {};
		rtResolveColorAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_RESOLVE_COLOR_ATTACHMENT_BIND_POINT;
		rtResolveColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference swapchainInputAttachmentRef = {};
		swapchainInputAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_RESOLVE_COLOR_ATTACHMENT_BIND_POINT;
		swapchainInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference rtDepthAttachmentRef = {};
		rtDepthAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT;
		rtDepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription renderToTargetSubpass = {};
		renderToTargetSubpass.colorAttachmentCount = 1;
		renderToTargetSubpass.pColorAttachments = &rtColorAttachmentRef;
		renderToTargetSubpass.pDepthStencilAttachment = &rtDepthAttachmentRef;
		renderToTargetSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		renderToTargetSubpass.pResolveAttachments = &rtResolveColorAttachmentRef;

		VkSubpassDescription copySubpass = {};
		copySubpass.colorAttachmentCount = 1;
		copySubpass.pColorAttachments = &swapchainAttachmentRef;
		copySubpass.inputAttachmentCount = 1;
		copySubpass.pInputAttachments = &swapchainInputAttachmentRef;
		copySubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		std::array<VkAttachmentDescription, TO_SIZE_T(ForwardRenderer::FORWARD_ATTACHMENT_BIND_POINT_COUNT)> attachments{
			swapchainAttachment,
			rtColorAttachment,
			rtDepthAttachment,
			rtResolveColorAttachment
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
		rtColorAttachmentImageCI.samples = msaaSamples;
		rtColorAttachmentImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		rtColorAttachmentImageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
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

		VkImageCreateInfo rtDepthAttachmentImageCI = {};
		rtDepthAttachmentImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		rtDepthAttachmentImageCI.imageType = VK_IMAGE_TYPE_2D;
		rtDepthAttachmentImageCI.format = depthImageFormat;
		rtDepthAttachmentImageCI.extent = { swapchainExtent.width, swapchainExtent.height, 1 };
		rtDepthAttachmentImageCI.mipLevels = 1;
		rtDepthAttachmentImageCI.arrayLayers = 1;
		rtDepthAttachmentImageCI.samples = msaaSamples;
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


		// Resolve MSAA
		VkImageCreateInfo rtResolveColorAttachmentImageCI = {};
		rtResolveColorAttachmentImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		rtResolveColorAttachmentImageCI.imageType = VK_IMAGE_TYPE_2D;
		rtResolveColorAttachmentImageCI.format = swapchainImageFormat;
		rtResolveColorAttachmentImageCI.extent = { swapchainExtent.width, swapchainExtent.height, 1 };
		rtResolveColorAttachmentImageCI.mipLevels = 1;
		rtResolveColorAttachmentImageCI.arrayLayers = 1;
		rtResolveColorAttachmentImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		rtResolveColorAttachmentImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		rtResolveColorAttachmentImageCI.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		rtResolveColorAttachmentImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		rtResolveColorAttachmentImageCI.queueFamilyIndexCount = 1;
		rtResolveColorAttachmentImageCI.pQueueFamilyIndices = &graphicsQueueIndex;
		rtResolveColorAttachmentImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		CHECK_VK(vkCreateImage(device, &rtResolveColorAttachmentImageCI, nullptr, &forward.rtResolveColorAttachmentImage));

		vkGetImageMemoryRequirements(device, forward.rtResolveColorAttachmentImage, &memoryRequirements);
		rtAttachmentImageAI.allocationSize = memoryRequirements.size;
		rtAttachmentImageAI.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		CHECK_VK(vkAllocateMemory(device, &rtAttachmentImageAI, nullptr, &forward.rtResolveColorAttachmentMemory));
		CHECK_VK(vkBindImageMemory(device, forward.rtResolveColorAttachmentImage, forward.rtResolveColorAttachmentMemory, 0));

		VkImageViewCreateInfo rtResolveColorAttachmentImageViewCI = {};
		rtResolveColorAttachmentImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		rtResolveColorAttachmentImageViewCI.image = forward.rtResolveColorAttachmentImage;
		rtResolveColorAttachmentImageViewCI.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		rtResolveColorAttachmentImageViewCI.format = swapchainImageFormat;
		rtResolveColorAttachmentImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		rtResolveColorAttachmentImageViewCI.subresourceRange = swapchainImageSubresourceRange;

		CHECK_VK(vkCreateImageView(device, &rtResolveColorAttachmentImageViewCI, nullptr, &forward.rtResolveColorAttachmentImageView));

		CommandTransitionImageLayout(forward.rtResolveColorAttachmentImage, swapchainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


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
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_RESOLVE_COLOR_ATTACHMENT_BIND_POINT)] = forward.rtResolveColorAttachmentImageView;

			CHECK_VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &forward.frameBuffers[i]));
		}
	}

	void RHI::InitForwardDescriptorPool() noexcept
	{
		VkDescriptorPoolSize blitInputDescriptorPoolSize = {};
		blitInputDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		blitInputDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize rtViewProjUniformDescriptorPoolSize = {};
		rtViewProjUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		rtViewProjUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize lightsUniformDescriptorPoolSize = {};
		lightsUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightsUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize irradianceMapDescriptorPoolSize = {};
		irradianceMapDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		irradianceMapDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize envMapSamplerDescriptorPoolSize = {};
		envMapSamplerDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		envMapSamplerDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize envMapUniformDescriptorPoolSize = {};
		envMapUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		envMapUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		std::array<VkDescriptorPoolSize, 6> descriptorPoolSizes = 
		{ 
			blitInputDescriptorPoolSize, 
			rtViewProjUniformDescriptorPoolSize, 
			lightsUniformDescriptorPoolSize, 
			irradianceMapDescriptorPoolSize,
			envMapSamplerDescriptorPoolSize,
			envMapUniformDescriptorPoolSize
		};

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = TO_UINT32_T(descriptorPoolSizes.size());
		descriptorPoolCI.pPoolSizes = descriptorPoolSizes.data();
		descriptorPoolCI.maxSets = swapchainImageCount * TO_UINT32_T(descriptorPoolSizes.size());

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
		blitGraphicsPipelineCI.vertexLayout = lux::VertexLayout::NO_VERTEX_LAYOUT;
		blitGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		blitGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		blitGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		blitGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		blitGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		blitGraphicsPipelineCI.disableMSAA = true;
		blitGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		blitGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		blitGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS;
		blitGraphicsPipelineCI.viewDescriptorSetLayoutBindings = { blitDescriptorSetLayoutBinding };

		CreateGraphicsPipeline(blitGraphicsPipelineCI, forward.blitGraphicsPipeline);


		// Render Target Graphics Pipeline
		
		// View Layout
		VkDescriptorSetLayoutBinding rtViewProjDescriptorSetLayoutBinding = {};
		rtViewProjDescriptorSetLayoutBinding.binding = 0;
		rtViewProjDescriptorSetLayoutBinding.descriptorCount = 1;
		rtViewProjDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		rtViewProjDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding lightDescriptorSetLayoutBinding = {};
		lightDescriptorSetLayoutBinding.binding = 1;
		lightDescriptorSetLayoutBinding.descriptorCount = 1;
		lightDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding irradianceMapDescriptorSetLayoutBinding = {};
		irradianceMapDescriptorSetLayoutBinding.binding = 2;
		irradianceMapDescriptorSetLayoutBinding.descriptorCount = 1;
		irradianceMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		irradianceMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Material Layout
		VkDescriptorSetLayoutBinding materialParametersDescriptorSetLayoutBinding = {};
		materialParametersDescriptorSetLayoutBinding.binding = 0;
		materialParametersDescriptorSetLayoutBinding.descriptorCount = 1;
		materialParametersDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		materialParametersDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding materialAlbedoDescriptorSetLayoutBinding = {};
		materialAlbedoDescriptorSetLayoutBinding.binding = 1;
		materialAlbedoDescriptorSetLayoutBinding.descriptorCount = 1;
		materialAlbedoDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		materialAlbedoDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


		VkDescriptorSetLayoutBinding materialNormalDescriptorSetLayoutBinding = {};
		materialNormalDescriptorSetLayoutBinding.binding = 2;
		materialNormalDescriptorSetLayoutBinding.descriptorCount = 1;
		materialNormalDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		materialNormalDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


		// Push Constant
		VkPushConstantRange rtModelPushConstantRange = {};
		rtModelPushConstantRange.offset = 0;
		rtModelPushConstantRange.size = sizeof(RtModelConstant);
		rtModelPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPushConstantRange lightCountPushConstantRange = {};
		lightCountPushConstantRange.offset = sizeof(RtModelConstant);
		lightCountPushConstantRange.size = sizeof(LightCountPushConstant);
		lightCountPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


		// Graphics Pipeline
		GraphicsPipelineCreateInfo rtGraphicsPipelineCI = {};
		rtGraphicsPipelineCI.renderPass = forward.renderPass;
		rtGraphicsPipelineCI.subpassIndex = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		rtGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLight.vert.spv";
		rtGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLight.frag.spv";
		rtGraphicsPipelineCI.vertexLayout = lux::VertexLayout::VERTEX_FULL_LAYOUT;
		rtGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		rtGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		rtGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		rtGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		rtGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rtGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		rtGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		rtGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS;
		rtGraphicsPipelineCI.viewDescriptorSetLayoutBindings = { rtViewProjDescriptorSetLayoutBinding, lightDescriptorSetLayoutBinding, irradianceMapDescriptorSetLayoutBinding };
		rtGraphicsPipelineCI.materialDescriptorSetLayoutBindings = { materialParametersDescriptorSetLayoutBinding, materialAlbedoDescriptorSetLayoutBinding, materialNormalDescriptorSetLayoutBinding };
		rtGraphicsPipelineCI.pushConstants = { rtModelPushConstantRange, lightCountPushConstantRange };

		CreateGraphicsPipeline(rtGraphicsPipelineCI, forward.rtGraphicsPipeline);


		// Env map Pipeline
		VkDescriptorSetLayoutBinding envMapViewProjDescriptorSetLayoutBinding = {};
		envMapViewProjDescriptorSetLayoutBinding.binding = 0;
		envMapViewProjDescriptorSetLayoutBinding.descriptorCount = 1;
		envMapViewProjDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		envMapViewProjDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding envMapSamplerDescriptorSetLayoutBinding = {};
		envMapSamplerDescriptorSetLayoutBinding.binding = 1;
		envMapSamplerDescriptorSetLayoutBinding.descriptorCount = 1;
		envMapSamplerDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		envMapSamplerDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		GraphicsPipelineCreateInfo envMapGraphicsPipelineCI = {};
		envMapGraphicsPipelineCI.renderPass = forward.renderPass;
		envMapGraphicsPipelineCI.subpassIndex = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		envMapGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/envMap/envMap.vert.spv";
		envMapGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/envMap/envMap.frag.spv";
		envMapGraphicsPipelineCI.vertexLayout = lux::VertexLayout::VERTEX_BASIC_LAYOUT;
		envMapGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		envMapGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		envMapGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		envMapGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		envMapGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		envMapGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		envMapGraphicsPipelineCI.enableDepthWrite = VK_FALSE;
		envMapGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		envMapGraphicsPipelineCI.viewDescriptorSetLayoutBindings = { envMapViewProjDescriptorSetLayoutBinding, envMapSamplerDescriptorSetLayoutBinding};
	
		CreateGraphicsPipeline(envMapGraphicsPipelineCI, forward.envMapGraphicsPipeline);
	}

	void RHI::InitForwardSampler() noexcept
	{
		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = VK_FILTER_LINEAR;
		samplerCI.minFilter = VK_FILTER_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.anisotropyEnable = VK_FALSE;
		samplerCI.maxAnisotropy = 16;
		samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCI.unnormalizedCoordinates = VK_FALSE;
		samplerCI.compareEnable = VK_FALSE;
		samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 0.0f;

		CHECK_VK(vkCreateSampler(device, &samplerCI, nullptr, &forward.sampler));


		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;

		CHECK_VK(vkCreateSampler(device, &samplerCI, nullptr, &forward.cubemapSampler));
	}

	void RHI::InitForwardDescriptorSets() noexcept
	{
		// Allocate Blit Descriptor Sets
		std::vector<VkDescriptorSetLayout> blitDescriptorSetLayout(swapchainImageCount, forward.blitGraphicsPipeline.viewDescriptorSetLayout);
		VkDescriptorSetAllocateInfo blitDescriptorSetAI = {};
		blitDescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		blitDescriptorSetAI.descriptorPool = forward.descriptorPool;
		blitDescriptorSetAI.descriptorSetCount = swapchainImageCount;
		blitDescriptorSetAI.pSetLayouts = blitDescriptorSetLayout.data();

		forward.blitDescriptorSets.resize(TO_SIZE_T(swapchainImageCount));
		CHECK_VK(vkAllocateDescriptorSets(device, &blitDescriptorSetAI, forward.blitDescriptorSets.data()));
	
	
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
			blitDescriptorImageInfo.imageView = forward.rtResolveColorAttachmentImageView;
			blitWriteDescriptorSet.dstSet = forward.blitDescriptorSets[i];
		
			vkUpdateDescriptorSets(device, 1, &blitWriteDescriptorSet, 0, nullptr);
		}


		// Allocate Render Target Descriptor Set
		std::vector<VkDescriptorSetLayout> rtViewDescriptorSetLayout(swapchainImageCount, forward.rtGraphicsPipeline.viewDescriptorSetLayout);
		VkDescriptorSetAllocateInfo rtViewDescriptorSetAI = {};
		rtViewDescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		rtViewDescriptorSetAI.descriptorPool = forward.descriptorPool;
		rtViewDescriptorSetAI.descriptorSetCount = swapchainImageCount;
		rtViewDescriptorSetAI.pSetLayouts = rtViewDescriptorSetLayout.data();

		forward.rtViewDescriptorSets.resize(TO_SIZE_T(swapchainImageCount));
		CHECK_VK(vkAllocateDescriptorSets(device, &rtViewDescriptorSetAI, forward.rtViewDescriptorSets.data()));


		std::vector<VkDescriptorSetLayout> rtModelDescriptorSetLayout(swapchainImageCount, forward.rtGraphicsPipeline.modelDescriptorSetLayout);
		VkDescriptorSetAllocateInfo rtModelDescriptorSetAI = {};
		rtModelDescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		rtModelDescriptorSetAI.descriptorPool = forward.descriptorPool;
		rtModelDescriptorSetAI.descriptorSetCount = swapchainImageCount;
		rtModelDescriptorSetAI.pSetLayouts = rtModelDescriptorSetLayout.data();

		forward.rtModelDescriptorSets.resize(TO_SIZE_T(swapchainImageCount));
		CHECK_VK(vkAllocateDescriptorSets(device, &rtModelDescriptorSetAI, forward.rtModelDescriptorSets.data()));


		// Update Render Target Descriptor Sets

		// ViewProj UBO
		VkWriteDescriptorSet rtWriteViewProjDescriptorSet = {};
		rtWriteViewProjDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		rtWriteViewProjDescriptorSet.descriptorCount = 1;
		rtWriteViewProjDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		rtWriteViewProjDescriptorSet.dstBinding = 0;
		rtWriteViewProjDescriptorSet.dstArrayElement = 0;

		VkDescriptorBufferInfo rtViewProjDescriptorBufferInfo = {};
		rtViewProjDescriptorBufferInfo.offset = 0;
		rtViewProjDescriptorBufferInfo.range = sizeof(RtViewProjUniform);

		// Light UBO
		VkWriteDescriptorSet writeLightDescriptorSet = {};
		writeLightDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeLightDescriptorSet.descriptorCount = 1;
		writeLightDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeLightDescriptorSet.dstBinding = 1;
		writeLightDescriptorSet.dstArrayElement = 0;

		VkDescriptorBufferInfo lightDescriptorBufferInfo = {};
		lightDescriptorBufferInfo.offset = 0;
		lightDescriptorBufferInfo.range = sizeof(LightBuffer) * LIGHT_MAX_COUNT;


		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			rtViewProjDescriptorBufferInfo.buffer = forward.viewProjUniformBuffers[i].buffer;

			rtWriteViewProjDescriptorSet.pBufferInfo = &rtViewProjDescriptorBufferInfo;
			rtWriteViewProjDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];


			lightDescriptorBufferInfo.buffer = lightUniformBuffers[i].buffer;
			
			writeLightDescriptorSet.pBufferInfo = &lightDescriptorBufferInfo;
			writeLightDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];


			std::array<VkWriteDescriptorSet, 2> writeDescriptorSets = { rtWriteViewProjDescriptorSet, writeLightDescriptorSet };
			vkUpdateDescriptorSets(device, TO_UINT32_T(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void RHI::InitForwardUniformBuffers() noexcept
	{
		BufferCreateInfo viewProjUniformBufferCI = {};
		viewProjUniformBufferCI.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		viewProjUniformBufferCI.size = sizeof(RtViewProjUniform);
		viewProjUniformBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		viewProjUniformBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		forward.viewProjUniformBuffers.resize(TO_UINT32_T(swapchainImageCount));
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			CreateBuffer(viewProjUniformBufferCI, forward.viewProjUniformBuffers[i]);
		}
	}

	void RHI::UpdateForwardUniformBuffers(const scene::CameraNode* camera, const std::vector<resource::Material*>& materials, const std::vector<scene::LightNode*>& lights) noexcept
	{
		size_t lightCount = std::min(lights.size(), TO_SIZE_T(LIGHT_MAX_COUNT));
		
		if (lightCount != lightCountPushConstant.lightCount)
		{
			lightCountPushConstant.lightCount = TO_UINT32_T(lightCount);
		}


		RtViewProjUniform viewProj;

		viewProj.view = camera->GetViewTransform();
		viewProj.projection = camera->GetPerspectiveProjectionTransform();

		UpdateBuffer(forward.viewProjUniformBuffers[currentFrame], &viewProj);



		for (size_t i = 0; i < materials.size(); i++)
		{
			UpdateBuffer(materials[i]->buffer[currentFrame], &materials[i]->parameter);
		}

		// Lights

		std::vector<LightBuffer> lightDatas(lightCount);
		scene::LightNode* currentNode;
		for (size_t i = 0; i < lightCount; i++)
		{
			currentNode = lights[i];
			lightDatas[i].position = glm::vec4(currentNode->GetWorldPosition(), TO_UINT32_T(currentNode->GetType()));
			lightDatas[i].color = currentNode->GetColor();
		}

		UpdateBuffer(lightUniformBuffers[currentFrame], lightDatas.data());
	}

	// TODO: Ref sur le vecteur de meshes ?
	void RHI::RenderForward(const scene::CameraNode* camera, const std::vector<scene::MeshNode*> meshes, const std::vector<scene::LightNode*>& lights) noexcept
	{
		// Acquire next image
		VkFence* fence = &fences[currentFrame];
		VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

		vkWaitForFences(device, 1, fence, false, UINT64_MAX);
		vkResetFences(device, 1, fence);
		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkSemaphore* acquireSemaphore = &acquireSemaphores[currentFrame];
		VkSemaphore* presentSemaphore = &presentSemaphores[currentFrame];

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

		// Sort mesh node by material
		std::map<std::string, std::vector<scene::MeshNode*>> sortedMeshNodes;
		std::vector<resource::Material*> materials;
		{
			std::vector<scene::MeshNode*>::const_iterator it = meshes.cbegin();
			std::vector<scene::MeshNode*>::const_iterator itE = meshes.cend();

			for (; it != itE; ++it)
			{
				std::string key = (*it)->GetMaterial().name;
				
				std::map<std::string, std::vector<scene::MeshNode*>>::iterator firstMaterial = sortedMeshNodes.find(key);
				if(firstMaterial == sortedMeshNodes.end())
					materials.push_back(&(*it)->GetMaterial());

				sortedMeshNodes[(*it)->GetMaterial().name].push_back(*it);
			}
		}

		UpdateForwardUniformBuffers(camera, materials, lights);

		// Render Target Subpass
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipelineLayout, ForwardRenderer::FORWARD_VIEW_DESCRIPTOR_SET_LAYOUT, 1, &forward.rtViewDescriptorSets[currentFrame], 0, nullptr);

		vkCmdPushConstants(commandBuffer, forward.rtGraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(RtModelConstant), sizeof(LightCountPushConstant), &lightCountPushConstant);

		VkDeviceSize offset[] = { 0 };


		sortedMeshNodesConstIterator it = sortedMeshNodes.cbegin();
		sortedMeshNodesConstIterator itE = sortedMeshNodes.cend();
		
		for (; it != itE; ++it)
		{
			std::vector<scene::MeshNode*> meshNodes = it->second;
			const resource::Material& material = meshNodes[0]->GetMaterial();

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipelineLayout, ForwardRenderer::FORWARD_MATERIAL_DESCRIPTOR_SET_LAYOUT, 1, &material.descriptorSet[currentFrame], 0, nullptr);


			std::vector<scene::MeshNode*>::const_iterator itMesh = meshNodes.cbegin();
			std::vector<scene::MeshNode*>::const_iterator itMeshEnd = meshNodes.cend();
		
			for (; itMesh != itMeshEnd; ++itMesh)
			{
				forward.modelConstant.model = (*itMesh)->GetWorldTransform();
				vkCmdPushConstants(commandBuffer, forward.rtGraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RtModelConstant), &forward.modelConstant);

				const resource::Mesh& currentMesh = (*itMesh)->GetMesh();
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &currentMesh.vertexBuffer.buffer, offset);
				vkCmdBindIndexBuffer(commandBuffer, currentMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, currentMesh.indexCount, 1, 0, 0, 0);
			}
		}


		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.envMapGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.envMapGraphicsPipeline.pipelineLayout, 0, 1, &forward.envMapViewDescriptorSets[currentFrame], 0, nullptr);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->vertexBuffer.buffer, offset);
		vkCmdBindIndexBuffer(commandBuffer, cube->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, cube->indexCount, 1, 0, 0, 0);

		RenderImgui();

		// Blit Subpass
		vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.blitGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.blitGraphicsPipeline.pipelineLayout, 0, 1, &forward.blitDescriptorSets[currentFrame], 0, nullptr);

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

	void RHI::RebuildForwardGraphicsPipeline() noexcept
	{
		vkDeviceWaitIdle(device);
		DestroyForwardGraphicsPipeline();
		InitForwardGraphicsPipelines();
	}
	
	void RHI::DestroyForwardGraphicsPipeline() noexcept
	{
		DestroyGraphicsPipeline(forward.blitGraphicsPipeline);
		DestroyGraphicsPipeline(forward.rtGraphicsPipeline);
		DestroyGraphicsPipeline(forward.envMapGraphicsPipeline);
	}

	void RHI::DestroyForwardRenderer() noexcept
	{
		DestroyForwardGraphicsPipeline();

		vkDestroyDescriptorPool(device, forward.descriptorPool, nullptr);

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			DestroyBuffer(forward.viewProjUniformBuffers[i]);
			vkDestroyFramebuffer(device, forward.frameBuffers[i], nullptr);

			vkDestroyImage(device, forward.rtColorAttachmentImages[i], nullptr);
			vkDestroyImageView(device, forward.rtColorAttachmentImageViews[i], nullptr);
			vkFreeMemory(device, forward.rtColorAttachmentImageMemories[i], nullptr);
		}
		
		vkDestroySampler(device, forward.sampler, nullptr);
		vkDestroySampler(device, forward.cubemapSampler, nullptr);

		vkDestroyImage(device, forward.rtResolveColorAttachmentImage, nullptr);
		vkDestroyImageView(device, forward.rtResolveColorAttachmentImageView, nullptr);
		vkFreeMemory(device, forward.rtResolveColorAttachmentMemory, nullptr);

		vkDestroyImage(device, forward.rtDepthAttachmentImage, nullptr);
		vkDestroyImageView(device, forward.rtDepthAttachmentImageView, nullptr);
		vkFreeMemory(device, forward.rtDepthAttachmentMemory, nullptr);

		vkDestroyRenderPass(device, forward.renderPass, nullptr);
	}

} // namespace lux::rhi