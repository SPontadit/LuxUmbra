#include "rhi\RHI.h"

#include <array>
#include <map>
#include <chrono>

#include "glm\gtc\matrix_transform.hpp"


namespace lux::rhi
{
	using sortedMeshNodesIterator = std::map<std::string, std::vector<scene::MeshNode*>>::iterator;
	using sortedMeshNodesConstIterator = std::map<std::string, std::vector<scene::MeshNode*>>::const_iterator;
	
	ForwardRenderer::ForwardRenderer() noexcept
		: rtImageFormat(VK_FORMAT_R32G32B32A32_SFLOAT), rtRenderPass(VK_NULL_HANDLE), rtFrameBuffers(0), descriptorPool(VK_NULL_HANDLE),
		blitRenderPass(VK_NULL_HANDLE), blitFrameBuffers(0), blitGraphicsPipeline(), blitDescriptorSets(0),
		rtGraphicsPipeline(), rtViewDescriptorSets(0), rtModelDescriptorSets(0), rtColorAttachmentImages(0), rtColorAttachmentImageMemories(0), rtColorAttachmentImageViews(0),
		rtResolveColorAttachmentImage(VK_NULL_HANDLE), rtResolveColorAttachmentMemory(VK_NULL_HANDLE), rtResolveColorAttachmentImageView(VK_NULL_HANDLE),
		rtDepthAttachmentImage(VK_NULL_HANDLE), rtDepthAttachmentMemory(VK_NULL_HANDLE), rtDepthAttachmentImageView(VK_NULL_HANDLE),
		envMapGraphicsPipeline(), envMapViewDescriptorSets(0), modelConstant(), viewProjUniformBuffers(0),
		sampler(VK_NULL_HANDLE), cubemapSampler(VK_NULL_HANDLE), irradianceSampler(VK_NULL_HANDLE), prefilteredSampler(VK_NULL_HANDLE),
		rtCutoutGraphicsPipeline(), rtTransparentBackGraphicsPipeline(), rtTransparentFrontGraphicsPipeline()
	{

	}

	void RHI::InitForwardRenderPass() noexcept
	{
		VkAttachmentDescription rtColorAttachment = {};
		rtColorAttachment.format = forward.rtImageFormat;
		rtColorAttachment.samples = msaaSamples;
		rtColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription rtDepthAttachment = {};
		rtDepthAttachment.format = depthImageFormat;
		rtDepthAttachment.samples = msaaSamples;
		rtDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtDepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtDepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtDepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtDepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentDescription rtPositionAttachment = {};
		rtPositionAttachment.format = forward.rtImageFormat;
		rtPositionAttachment.samples = msaaSamples;
		rtPositionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtPositionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtPositionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtPositionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtPositionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtPositionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription rtNormalAttachment = {};
		rtNormalAttachment.format = forward.rtImageFormat;
		rtNormalAttachment.samples = msaaSamples;
		rtNormalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtNormalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtNormalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtNormalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtNormalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtNormalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription rtResolveColorAttachment = {};
		rtResolveColorAttachment.format = forward.rtImageFormat;
		rtResolveColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		rtResolveColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtResolveColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtResolveColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtResolveColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription rtResolvePositionAttachment = {};
		rtResolvePositionAttachment.format = forward.rtImageFormat;
		rtResolvePositionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		rtResolvePositionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolvePositionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtResolvePositionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolvePositionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtResolvePositionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtResolvePositionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription rtResolveNormalAttachment = {};
		rtResolveNormalAttachment.format = forward.rtImageFormat;
		rtResolveNormalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		rtResolveNormalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveNormalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtResolveNormalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveNormalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtResolveNormalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtResolveNormalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription rtResolveDepthAttachment = {};
		rtResolveDepthAttachment.format = depthImageFormat;
		rtResolveDepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		rtResolveDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtResolveDepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveDepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtResolveDepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtResolveDepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference rtColorAttachmentRef = {};
		rtColorAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT;
		rtColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtPositionAttachmentRef = {};
		rtPositionAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_POSITION_ATTACHMENT_BIND_POINT;
		rtPositionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtNormalAttachmentRef = {};
		rtNormalAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_NORMAL_ATTACHMENT_BIND_POINT;
		rtNormalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtDepthAttachmentRef = {};
		rtDepthAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT;
		rtDepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentReference, 3> colorAttachments = { rtColorAttachmentRef, rtPositionAttachmentRef, rtNormalAttachmentRef };

		VkSubpassDescription renderToTargetSubpass = {};
		renderToTargetSubpass.colorAttachmentCount = TO_UINT32_T(colorAttachments.size());
		renderToTargetSubpass.pColorAttachments = colorAttachments.data();
		renderToTargetSubpass.pDepthStencilAttachment = &rtDepthAttachmentRef;
		renderToTargetSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		//renderToTargetSubpass.pResolveAttachments = resolveAttachments.data();


		std::array<VkAttachmentDescription, TO_SIZE_T(ForwardRenderer::FORWARD_RT_ATTACHMENT_BIND_POINT_COUNT)> attachments{
			rtColorAttachment,
			rtDepthAttachment,
			rtPositionAttachment,
			rtNormalAttachment
		};

		//std::array<VkSubpassDescription, TO_SIZE_T(ForwardRenderer::FORWARD_SUBPASS_COUNT)> subpasses{
		//	renderToTargetSubpass,
		//};

		std::array<VkSubpassDependency, 2> subpassDependencies;
		subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[0].dstSubpass = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpassDependencies[1].srcSubpass = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


		VkRenderPassCreateInfo rtRenderPassCI = {};
		rtRenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rtRenderPassCI.attachmentCount = ForwardRenderer::FORWARD_RT_ATTACHMENT_BIND_POINT_COUNT;
		rtRenderPassCI.pAttachments = attachments.data();
		rtRenderPassCI.subpassCount = 1;
		rtRenderPassCI.pSubpasses = &renderToTargetSubpass;
		rtRenderPassCI.dependencyCount = TO_UINT32_T(subpassDependencies.size());
		rtRenderPassCI.pDependencies = subpassDependencies.data();

		CHECK_VK(vkCreateRenderPass(device, &rtRenderPassCI, nullptr, &forward.rtRenderPass));


		VkAttachmentDescription swapchainAttachment = {};
		swapchainAttachment.format = swapchainImageFormat;
		swapchainAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		swapchainAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		swapchainAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		swapchainAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		swapchainAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		swapchainAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		swapchainAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference swapchainAttachmentRef = {};
		swapchainAttachmentRef.attachment = ForwardRenderer::FORWARD_SWAPCHAIN_COLOR_ATTACHMENT_BIND_POINT;
		swapchainAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription copySubpass = {};
		copySubpass.colorAttachmentCount = 1;
		copySubpass.pColorAttachments = &swapchainAttachmentRef;
		copySubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkRenderPassCreateInfo blitRenderPassCI = {};
		blitRenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		blitRenderPassCI.attachmentCount = 1;
		blitRenderPassCI.pAttachments = &swapchainAttachment;
		blitRenderPassCI.subpassCount = 1;
		blitRenderPassCI.pSubpasses = &copySubpass;

		CHECK_VK(vkCreateRenderPass(device, &blitRenderPassCI, nullptr, &forward.blitRenderPass));
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
		rtColorAttachmentImageCI.format = forward.rtImageFormat;
		rtColorAttachmentImageCI.extent = { swapchainExtent.width, swapchainExtent.height, 1 };
		rtColorAttachmentImageCI.mipLevels = 1;
		rtColorAttachmentImageCI.arrayLayers = 1;
		rtColorAttachmentImageCI.samples = msaaSamples;
		rtColorAttachmentImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		rtColorAttachmentImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		rtColorAttachmentImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		rtColorAttachmentImageCI.queueFamilyIndexCount = 1;
		rtColorAttachmentImageCI.pQueueFamilyIndices = &graphicsQueueIndex;
		rtColorAttachmentImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageViewCreateInfo rtColorAttachmentImageViewCI = {};
		rtColorAttachmentImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		rtColorAttachmentImageViewCI.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		rtColorAttachmentImageViewCI.format = forward.rtImageFormat;
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
		rtDepthAttachmentImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

		// Position Map
		ImageCreateInfo rtPositionImageCI = {};
		rtPositionImageCI.format = forward.rtImageFormat;
		rtPositionImageCI.width = swapchainExtent.width;
		rtPositionImageCI.height = swapchainExtent.height;
		rtPositionImageCI.arrayLayers = 1;
		rtPositionImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		rtPositionImageCI.sampleCount = msaaSamples;
		rtPositionImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		rtPositionImageCI.subresourceRangeLayerCount = 1;
		rtPositionImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(rtPositionImageCI, forward.rtPositionMap);

		// Normal Map
		ImageCreateInfo rtNormalImageCI = {};
		rtNormalImageCI.format = forward.rtImageFormat;
		rtNormalImageCI.width = swapchainExtent.width;
		rtNormalImageCI.height = swapchainExtent.height;
		rtNormalImageCI.arrayLayers = 1;
		rtNormalImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		rtNormalImageCI.sampleCount = msaaSamples;
		rtNormalImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		rtNormalImageCI.subresourceRangeLayerCount = 1;
		rtNormalImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(rtNormalImageCI, forward.rtNormalMap);


		// Framebuffers

		std::array<VkImageView, TO_SIZE_T(ForwardRenderer::FORWARD_RT_ATTACHMENT_BIND_POINT_COUNT)> attachments { VK_NULL_HANDLE };

		VkFramebufferCreateInfo rtFramebufferCI = {};
		rtFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		rtFramebufferCI.renderPass = forward.rtRenderPass;
		rtFramebufferCI.width = swapchainExtent.width;
		rtFramebufferCI.height = swapchainExtent.height;
		rtFramebufferCI.layers = 1;
		rtFramebufferCI.attachmentCount = ForwardRenderer::FORWARD_RT_ATTACHMENT_BIND_POINT_COUNT;
		rtFramebufferCI.pAttachments = attachments.data();

		VkFramebufferCreateInfo blitFramebufferCI = {};
		blitFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		blitFramebufferCI.renderPass = forward.blitRenderPass;
		blitFramebufferCI.width = swapchainExtent.width;
		blitFramebufferCI.height = swapchainExtent.height;
		blitFramebufferCI.layers = 1;
		blitFramebufferCI.attachmentCount = 1;

		forward.rtFrameBuffers.resize(TO_SIZE_T(swapchainImageCount));
		forward.blitFrameBuffers.resize(TO_SIZE_T(swapchainImageCount));

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			blitFramebufferCI.pAttachments = &swapchainImageViews[i];
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT)] = forward.rtColorAttachmentImageViews[i];
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT)] = forward.rtDepthAttachmentImageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_POSITION_ATTACHMENT_BIND_POINT)] = forward.rtPositionMap.imageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_NORMAL_ATTACHMENT_BIND_POINT)] = forward.rtNormalMap.imageView;

			CHECK_VK(vkCreateFramebuffer(device, &rtFramebufferCI, nullptr, &forward.rtFrameBuffers[i]));
			CHECK_VK(vkCreateFramebuffer(device, &blitFramebufferCI, nullptr, &forward.blitFrameBuffers[i]));
		}

	}

	void RHI::InitForwardDescriptorPool() noexcept
	{
		VkDescriptorPoolSize blitDescriptorPoolSize = {};
		blitDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		blitDescriptorPoolSize.descriptorCount = swapchainImageCount * 4;

		VkDescriptorPoolSize rtViewProjUniformDescriptorPoolSize = {};
		rtViewProjUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		rtViewProjUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize lightsUniformDescriptorPoolSize = {};
		lightsUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightsUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize shadowMapsDescriptorPoolSize = {};
		shadowMapsDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		shadowMapsDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize irradianceMapDescriptorPoolSize = {};
		irradianceMapDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		irradianceMapDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize prefilteredMapDescriptorPoolSize = {};
		prefilteredMapDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		prefilteredMapDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize BRDFLutMapDescriptorPoolSize = {};
		BRDFLutMapDescriptorPoolSize .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		BRDFLutMapDescriptorPoolSize .descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize envMapSamplerDescriptorPoolSize = {};
		envMapSamplerDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		envMapSamplerDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize envMapUniformDescriptorPoolSize = {};
		envMapUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		envMapUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		std::array<VkDescriptorPoolSize, 9> descriptorPoolSizes = 
		{ 
			blitDescriptorPoolSize,
			rtViewProjUniformDescriptorPoolSize, 
			lightsUniformDescriptorPoolSize, 
			shadowMapsDescriptorPoolSize,
			irradianceMapDescriptorPoolSize,
			prefilteredMapDescriptorPoolSize,
			BRDFLutMapDescriptorPoolSize,
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

	void RHI::InitForwardGraphicsPipelines(bool useCache) noexcept
	{

		// Blit Graphics Pipeline
		VkDescriptorSetLayoutBinding blitDescriptorSetLayoutBinding = {};
		blitDescriptorSetLayoutBinding.binding = 0;
		blitDescriptorSetLayoutBinding.descriptorCount = 1;
		blitDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		blitDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding positionMapDescriptorSetLayoutBinding = {};
		positionMapDescriptorSetLayoutBinding.binding = 1;
		positionMapDescriptorSetLayoutBinding.descriptorCount = 1;
		positionMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		positionMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding normalMapDescriptorSetLayoutBinding = {};
		normalMapDescriptorSetLayoutBinding.binding = 2;
		normalMapDescriptorSetLayoutBinding.descriptorCount = 1;
		normalMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding depthMapDescriptorSetLayoutBinding = {};
		depthMapDescriptorSetLayoutBinding.binding = 3;
		depthMapDescriptorSetLayoutBinding.descriptorCount = 1;
		depthMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		depthMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPushConstantRange blitPostProcessParameterPushConstantRange = {};
		blitPostProcessParameterPushConstantRange.offset = 0;
		blitPostProcessParameterPushConstantRange.size = sizeof(PostProcessParameters);
		blitPostProcessParameterPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		forward.postProcessParameters.inverseScreenSize = { TO_FLOAT(1.0 / swapchainExtent.width), TO_FLOAT(1.0 / swapchainExtent.height) };

		GraphicsPipelineCreateInfo blitGraphicsPipelineCI = {};
		blitGraphicsPipelineCI.renderPass = forward.blitRenderPass;
		blitGraphicsPipelineCI.subpassIndex = 0;
		blitGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/blit/blit.vert.spv";
		blitGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/blit/blit.frag.spv";
		blitGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/blitGraphics.bin";
		blitGraphicsPipelineCI.vertexLayout = lux::VertexLayout::NO_VERTEX_LAYOUT;
		blitGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		blitGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		blitGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		blitGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		blitGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		blitGraphicsPipelineCI.disableMSAA = true;
		blitGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		blitGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		blitGraphicsPipelineCI.enableDepthBias = VK_FALSE;
		blitGraphicsPipelineCI.depthBiasConstantFactor = 0.f;
		blitGraphicsPipelineCI.depthBiasSlopeFactor = 0.f;
		blitGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS;
		blitGraphicsPipelineCI.viewDescriptorSetLayoutBindings = 
		{ 
			blitDescriptorSetLayoutBinding,
			positionMapDescriptorSetLayoutBinding,
			normalMapDescriptorSetLayoutBinding,
			depthMapDescriptorSetLayoutBinding
		};
		
		blitGraphicsPipelineCI.pushConstants = { blitPostProcessParameterPushConstantRange };

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

		VkDescriptorSetLayoutBinding prefilteredMapDescriptorSetLayoutBinding = {};
		prefilteredMapDescriptorSetLayoutBinding.binding = 3;
		prefilteredMapDescriptorSetLayoutBinding.descriptorCount = 1;
		prefilteredMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		prefilteredMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding BRDFLutDescriptorSetLayoutBinding = {};
		BRDFLutDescriptorSetLayoutBinding.binding = 4;
		BRDFLutDescriptorSetLayoutBinding.descriptorCount = 1;
		BRDFLutDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		BRDFLutDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding shadowMapDescriptorSetLayoutBinding = {};
		shadowMapDescriptorSetLayoutBinding.binding = 5;
		shadowMapDescriptorSetLayoutBinding.descriptorCount = 1;
		shadowMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		shadowMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


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

		VkDescriptorSetLayoutBinding materialMetallicRoughnessDescriptorSetLayoutBinding = {};
		materialMetallicRoughnessDescriptorSetLayoutBinding.binding = 3;
		materialMetallicRoughnessDescriptorSetLayoutBinding.descriptorCount = 1;
		materialMetallicRoughnessDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		materialMetallicRoughnessDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding materialAmbientOcclusionDescriptorSetLayoutBinding = {};
		materialAmbientOcclusionDescriptorSetLayoutBinding.binding = 4;
		materialAmbientOcclusionDescriptorSetLayoutBinding.descriptorCount = 1;
		materialAmbientOcclusionDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		materialAmbientOcclusionDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


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
		rtGraphicsPipelineCI.renderPass = forward.rtRenderPass;
		rtGraphicsPipelineCI.subpassIndex = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		rtGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLight.vert.spv";
		rtGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLight.frag.spv";
		rtGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtGraphics.bin";
		rtGraphicsPipelineCI.vertexLayout = lux::VertexLayout::VERTEX_FULL_LAYOUT;
		rtGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		rtGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		rtGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		rtGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		rtGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rtGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		rtGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		rtGraphicsPipelineCI.enableDepthBias = VK_FALSE;
		rtGraphicsPipelineCI.depthBiasConstantFactor = 0.f;
		rtGraphicsPipelineCI.depthBiasSlopeFactor = 0.f;
		rtGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS;
		rtGraphicsPipelineCI.colorBlendAttachmentStateCount = 3;

		rtGraphicsPipelineCI.viewDescriptorSetLayoutBindings = 
		{ 
			rtViewProjDescriptorSetLayoutBinding, 
			lightDescriptorSetLayoutBinding, 
			irradianceMapDescriptorSetLayoutBinding, 
			prefilteredMapDescriptorSetLayoutBinding,
			BRDFLutDescriptorSetLayoutBinding,
			shadowMapDescriptorSetLayoutBinding,
		};

		rtGraphicsPipelineCI.materialDescriptorSetLayoutBindings = 
		{ 
			materialParametersDescriptorSetLayoutBinding, 
			materialAlbedoDescriptorSetLayoutBinding, 
			materialNormalDescriptorSetLayoutBinding,
			materialMetallicRoughnessDescriptorSetLayoutBinding,
			materialAmbientOcclusionDescriptorSetLayoutBinding,
		};

		rtGraphicsPipelineCI.pushConstants = { rtModelPushConstantRange, lightCountPushConstantRange };

		CreateGraphicsPipeline(rtGraphicsPipelineCI, forward.rtGraphicsPipeline);


		// Cutout Graphics Pipeline
		rtGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLightCutout.frag.spv";
		rtGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtCutoutGraphics.bin";
		rtGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		rtGraphicsPipelineCI.disableColorWriteMask = true;
		rtGraphicsPipelineCI.enableBlend = true;

		CreateGraphicsPipeline(rtGraphicsPipelineCI, forward.rtCutoutGraphicsPipeline);


		// Front Face Transparent Graphics Pipeline
		rtGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLight.frag.spv";
		rtGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtTransparentFrontGraphics.bin";
		rtGraphicsPipelineCI.disableColorWriteMask = false;
		rtGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		rtGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;

		CreateGraphicsPipeline(rtGraphicsPipelineCI, forward.rtTransparentFrontGraphicsPipeline);


		// Back Face Transparent Graphics Pipeline
		rtGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_FRONT_BIT;
		rtGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtTransparentBackGraphics.bin";

		CreateGraphicsPipeline(rtGraphicsPipelineCI, forward.rtTransparentBackGraphicsPipeline);


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
		envMapGraphicsPipelineCI.renderPass = forward.rtRenderPass;
		envMapGraphicsPipelineCI.subpassIndex = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		envMapGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/envMap/envMap.vert.spv";
		envMapGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/envMap/envMap.frag.spv";
		envMapGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtEnvMapGraphics.bin";
		envMapGraphicsPipelineCI.vertexLayout = lux::VertexLayout::VERTEX_BASIC_LAYOUT;
		envMapGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		envMapGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		envMapGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		envMapGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		envMapGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		envMapGraphicsPipelineCI.colorBlendAttachmentStateCount = 3;
		envMapGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		envMapGraphicsPipelineCI.enableDepthWrite = VK_FALSE;
		envMapGraphicsPipelineCI.enableDepthBias = VK_FALSE;
		envMapGraphicsPipelineCI.depthBiasConstantFactor = 0.f;
		envMapGraphicsPipelineCI.depthBiasSlopeFactor = 0.f;
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
		samplerCI.maxLod = TO_FLOAT(floor(log2(CUBEMAP_TEXTURE_SIZE))) + 1.0f;

		CHECK_VK(vkCreateSampler(device, &samplerCI, nullptr, &forward.cubemapSampler));


		samplerCI.maxLod = TO_FLOAT(floor(log2(IRRADIANCE_TEXTURE_SIZE))) + 1.0f;

		CHECK_VK(vkCreateSampler(device, &samplerCI, nullptr, &forward.irradianceSampler));


		samplerCI.maxLod = TO_FLOAT(floor(log2(PREFILTERED_TEXTURE_SIZE))) + 1.0f;
		CHECK_VK(vkCreateSampler(device, &samplerCI, nullptr, &forward.prefilteredSampler));

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
		blitDescriptorImageInfo.sampler = forward.sampler;

		VkWriteDescriptorSet writeBlitDescriptorSet = {};
		writeBlitDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeBlitDescriptorSet.descriptorCount = 1;
		writeBlitDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeBlitDescriptorSet.dstBinding = 0;
		writeBlitDescriptorSet.pImageInfo = &blitDescriptorImageInfo;


		VkDescriptorImageInfo positionMapDescriptorImageInfo = {};
		positionMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		positionMapDescriptorImageInfo.sampler = forward.sampler;
		positionMapDescriptorImageInfo.imageView = forward.rtPositionMap.imageView;

		VkWriteDescriptorSet writePositionMapDescriptorSet = {};
		writePositionMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writePositionMapDescriptorSet.descriptorCount = 1;
		writePositionMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writePositionMapDescriptorSet.dstBinding = 1;
		writePositionMapDescriptorSet.pImageInfo = &positionMapDescriptorImageInfo;


		VkDescriptorImageInfo normalMapDescriptorImageInfo = {};
		normalMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalMapDescriptorImageInfo.sampler = forward.sampler;
		normalMapDescriptorImageInfo.imageView = forward.rtNormalMap.imageView;

		VkWriteDescriptorSet writeNormalMapDescriptorSet = {};
		writeNormalMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeNormalMapDescriptorSet.descriptorCount = 1;
		writeNormalMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeNormalMapDescriptorSet.dstBinding = 2;
		writeNormalMapDescriptorSet.pImageInfo = &normalMapDescriptorImageInfo;


		VkDescriptorImageInfo depthMapDescriptorImageInfo = {};
		depthMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		depthMapDescriptorImageInfo.sampler = forward.sampler;
		depthMapDescriptorImageInfo.imageView = forward.rtDepthAttachmentImageView;

		VkWriteDescriptorSet writeDepthMapDescriptorSet = {};
		writeDepthMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDepthMapDescriptorSet.descriptorCount = 1;
		writeDepthMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDepthMapDescriptorSet.dstBinding = 3;
		writeDepthMapDescriptorSet.pImageInfo = &depthMapDescriptorImageInfo;
		
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			blitDescriptorImageInfo.imageView = forward.rtColorAttachmentImageViews[i];

			writeBlitDescriptorSet.dstSet = forward.blitDescriptorSets[i];
			writePositionMapDescriptorSet.dstSet = forward.blitDescriptorSets[i];
			writeNormalMapDescriptorSet.dstSet = forward.blitDescriptorSets[i];
			writeDepthMapDescriptorSet.dstSet = forward.blitDescriptorSets[i];
			
			std::array<VkWriteDescriptorSet, 4> writeDescriptorSets = 
			{
				writeBlitDescriptorSet,
				writePositionMapDescriptorSet,
				writeNormalMapDescriptorSet,
				writeDepthMapDescriptorSet
			};

			vkUpdateDescriptorSets(device, TO_UINT32_T(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
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
		VkDescriptorBufferInfo rtViewProjDescriptorBufferInfo = {};
		rtViewProjDescriptorBufferInfo.offset = 0;
		rtViewProjDescriptorBufferInfo.range = sizeof(RtViewProjUniform);

		VkWriteDescriptorSet rtWriteViewProjDescriptorSet = {};
		rtWriteViewProjDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		rtWriteViewProjDescriptorSet.descriptorCount = 1;
		rtWriteViewProjDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		rtWriteViewProjDescriptorSet.dstBinding = 0;
		rtWriteViewProjDescriptorSet.dstArrayElement = 0;
		rtWriteViewProjDescriptorSet.pBufferInfo = &rtViewProjDescriptorBufferInfo;

		// Light UBO
		VkDescriptorBufferInfo lightDescriptorBufferInfo = {};
		lightDescriptorBufferInfo.offset = 0;
		lightDescriptorBufferInfo.range = sizeof(LightBuffer) * LIGHT_MAX_COUNT;

		VkWriteDescriptorSet writeLightDescriptorSet = {};
		writeLightDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeLightDescriptorSet.descriptorCount = 1;
		writeLightDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeLightDescriptorSet.dstBinding = 1;
		writeLightDescriptorSet.dstArrayElement = 0;
		writeLightDescriptorSet.pBufferInfo = &lightDescriptorBufferInfo;

		// Shadow map sampler
		VkDescriptorImageInfo shadowMapImageDescriptorInfo = {};
		shadowMapImageDescriptorInfo.imageView = shadowMapper.shadowMap.imageView;
		shadowMapImageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		shadowMapImageDescriptorInfo.sampler = forward.sampler;

		VkWriteDescriptorSet writeShadowMapDescriptorSet = {};
		writeShadowMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeShadowMapDescriptorSet.descriptorCount = 1;
		writeShadowMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeShadowMapDescriptorSet.dstBinding = 5;
		writeShadowMapDescriptorSet.dstArrayElement = 0;
		writeShadowMapDescriptorSet.pImageInfo = &shadowMapImageDescriptorInfo;


		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			rtViewProjDescriptorBufferInfo.buffer = forward.viewProjUniformBuffers[i].buffer;
			rtWriteViewProjDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			lightDescriptorBufferInfo.buffer = lightUniformBuffers[i].buffer;
			writeLightDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			writeShadowMapDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets = { rtWriteViewProjDescriptorSet, writeLightDescriptorSet, writeShadowMapDescriptorSet };
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


		forward.rtViewProjUniform.view = camera->GetViewTransform();
		forward.rtViewProjUniform.projection = camera->GetPerspectiveProjectionTransform();

		UpdateBuffer(forward.viewProjUniformBuffers[currentFrame], &forward.rtViewProjUniform);



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

			if (currentNode->GetType() == scene::LightType::LIGHT_TYPE_DIRECTIONAL)
			{
				glm::vec3 worldRotation = glm::rotate(currentNode->GetWorldRotation(), glm::vec3(0.0f, 0.0f, -1.0f));
				lightDatas[i].position = glm::vec4(worldRotation, 0.0f);
			}
			else // if (currentNode->GetType() == scene::LightType::LIGHT_TYPE_POINT)
			{
				lightDatas[i].position = glm::vec4(currentNode->GetWorldPosition(), 1.0f);
			}
			
			lightDatas[i].color = currentNode->GetColor();
		}

		UpdateBuffer(lightUniformBuffers[currentFrame], lightDatas.data());
	}

	void RHI::RenderForward(VkCommandBuffer commandBuffer, int imageIndex, const scene::CameraNode* camera, const std::vector<scene::MeshNode*>& meshes, const std::vector<scene::LightNode*>& lights) noexcept
	{
		VkDeviceSize vertexBufferOffsets[] = { 0 };

		VkClearColorValue clearColor{ 0.5f, 0.5703125f, 0.6171875f, 1.0F };

		std::array<VkClearValue, 4> clearValues = {};
		clearValues[ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT].color = clearColor;
		clearValues[ForwardRenderer::FORWARD_RT_POSITION_ATTACHMENT_BIND_POINT].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[ForwardRenderer::FORWARD_RT_NORMAL_ATTACHMENT_BIND_POINT].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT].depthStencil = { 1.0f, 0 };


		// Begin Render Pass
		VkRenderPassBeginInfo renderPassBI = {};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.renderPass = forward.rtRenderPass;
		renderPassBI.framebuffer = forward.rtFrameBuffers[imageIndex];
		renderPassBI.renderArea.extent = swapchainExtent;
		renderPassBI.clearValueCount = TO_UINT32_T(clearValues.size());
		renderPassBI.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

		// Sort mesh node by material
		std::map<std::string, std::vector<scene::MeshNode*>> sortedMeshNodes;
		std::map<std::string, std::vector<scene::MeshNode*>> sortedTransparentMeshNodes;
		std::vector<resource::Material*> materials;
		{
			std::vector<scene::MeshNode*>::const_iterator it = meshes.cbegin();
			std::vector<scene::MeshNode*>::const_iterator itE = meshes.cend();

			for (; it != itE; ++it)
			{
				resource::Material* currentMaterial = &(*it)->GetMaterial();
				std::string key = currentMaterial->name;

				std::map<std::string, std::vector<scene::MeshNode*>>::iterator firstMaterial = sortedMeshNodes.find(key);
				std::map<std::string, std::vector<scene::MeshNode*>>::iterator firstTransparentMaterial = sortedTransparentMeshNodes.find(key);
				if (firstMaterial == sortedMeshNodes.end() || firstTransparentMaterial == sortedTransparentMeshNodes.end())
				{
					materials.push_back(currentMaterial);
				}

				if (currentMaterial->isTransparent)
				{
					//sortedMeshNodes[key].push_back(*it);
					sortedTransparentMeshNodes[key].push_back(*it);
				}
				else
					sortedMeshNodes[key].push_back(*it);
			}
		}

		UpdateForwardUniformBuffers(camera, materials, lights);

		// Render Target Subpass
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipelineLayout, ForwardRenderer::FORWARD_VIEW_DESCRIPTOR_SET_LAYOUT, 1, &forward.rtViewDescriptorSets[currentFrame], 0, nullptr);

		vkCmdPushConstants(commandBuffer, forward.rtGraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(RtModelConstant), sizeof(LightCountPushConstant), &lightCountPushConstant);

		sortedMeshNodesConstIterator it = sortedMeshNodes.cbegin();
		sortedMeshNodesConstIterator itE = sortedMeshNodes.cend();
		
		// Draw opaque object
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
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &currentMesh.vertexBuffer.buffer, vertexBufferOffsets);
				vkCmdBindIndexBuffer(commandBuffer, currentMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, currentMesh.indexCount, 1, 0, 0, 0);
			}
		}


		// Environment Map
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.envMapGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.envMapGraphicsPipeline.pipelineLayout, 0, 1, &forward.envMapViewDescriptorSets[currentFrame], 0, nullptr);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->vertexBuffer.buffer, vertexBufferOffsets);
		vkCmdBindIndexBuffer(commandBuffer, cube->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, cube->indexCount, 1, 0, 0, 0);



		// Draw transparent object
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipelineLayout, ForwardRenderer::FORWARD_VIEW_DESCRIPTOR_SET_LAYOUT, 1, &forward.rtViewDescriptorSets[currentFrame], 0, nullptr);

		it = sortedTransparentMeshNodes.cbegin();
		itE = sortedTransparentMeshNodes.cend();

		// TODO: Split transparent rendering into 3 loops - 1 per pipeline
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

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtCutoutGraphicsPipeline.pipeline);
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &currentMesh.vertexBuffer.buffer, vertexBufferOffsets);
				vkCmdBindIndexBuffer(commandBuffer, currentMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, currentMesh.indexCount, 1, 0, 0, 0);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtTransparentBackGraphicsPipeline.pipeline);
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &currentMesh.vertexBuffer.buffer, vertexBufferOffsets);
				vkCmdBindIndexBuffer(commandBuffer, currentMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, currentMesh.indexCount, 1, 0, 0, 0);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtTransparentFrontGraphicsPipeline.pipeline);
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &currentMesh.vertexBuffer.buffer, vertexBufferOffsets);
				vkCmdBindIndexBuffer(commandBuffer, currentMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, currentMesh.indexCount, 1, 0, 0, 0);
			}
		}

		vkCmdEndRenderPass(commandBuffer);
	}

	void RHI::RenderPostProcess(VkCommandBuffer commandBuffer, int imageIndex) noexcept
	{
		VkDeviceSize vertexBufferOffsets[] = { 0 };

		VkClearColorValue clearColor{ 0.5f, 0.5703125f, 0.6171875f, 1.0F };

		std::array<VkClearValue, 1> clearValues = {};
		clearValues[ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT].color = clearColor;

		// Begin Render Pass
		VkRenderPassBeginInfo renderPassBI = {};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.renderPass = forward.blitRenderPass;
		renderPassBI.framebuffer = forward.blitFrameBuffers[imageIndex];
		renderPassBI.renderArea.extent = swapchainExtent;
		renderPassBI.clearValueCount = TO_UINT32_T(clearValues.size());
		renderPassBI.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.blitGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.blitGraphicsPipeline.pipelineLayout, 0, 1, &forward.blitDescriptorSets[currentFrame], 0, nullptr);

		vkCmdPushConstants(commandBuffer, forward.blitGraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PostProcessParameters), &forward.postProcessParameters);

		vkCmdDraw(commandBuffer, 4, 1, 0, 0);

		RenderImgui();

		vkCmdEndRenderPass(commandBuffer);
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
		DestroyGraphicsPipeline(forward.rtCutoutGraphicsPipeline);
		DestroyGraphicsPipeline(forward.rtTransparentBackGraphicsPipeline);
		DestroyGraphicsPipeline(forward.rtTransparentFrontGraphicsPipeline);
		DestroyGraphicsPipeline(forward.envMapGraphicsPipeline);
	}

	void RHI::DestroyForwardRenderer() noexcept
	{
		DestroyForwardGraphicsPipeline();

		vkDestroyDescriptorPool(device, forward.descriptorPool, nullptr);

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			DestroyBuffer(forward.viewProjUniformBuffers[i]);
			vkDestroyFramebuffer(device, forward.rtFrameBuffers[i], nullptr);
			vkDestroyFramebuffer(device, forward.blitFrameBuffers[i], nullptr);

			vkDestroyImage(device, forward.rtColorAttachmentImages[i], nullptr);
			vkDestroyImageView(device, forward.rtColorAttachmentImageViews[i], nullptr);
			vkFreeMemory(device, forward.rtColorAttachmentImageMemories[i], nullptr);
		}
		
		vkDestroySampler(device, forward.sampler, nullptr);
		vkDestroySampler(device, forward.cubemapSampler, nullptr);
		vkDestroySampler(device, forward.irradianceSampler, nullptr);
		vkDestroySampler(device, forward.prefilteredSampler, nullptr);

		vkDestroyImage(device, forward.rtResolveColorAttachmentImage, nullptr);
		vkDestroyImageView(device, forward.rtResolveColorAttachmentImageView, nullptr);
		vkFreeMemory(device, forward.rtResolveColorAttachmentMemory, nullptr);

		vkDestroyImage(device, forward.rtDepthAttachmentImage, nullptr);
		vkDestroyImageView(device, forward.rtDepthAttachmentImageView, nullptr);
		vkFreeMemory(device, forward.rtDepthAttachmentMemory, nullptr);

		DestroyImage(forward.rtPositionMap);
		DestroyImage(forward.rtNormalMap);

		vkDestroyRenderPass(device, forward.rtRenderPass, nullptr);
		vkDestroyRenderPass(device, forward.blitRenderPass, nullptr);
	}

} // namespace lux::rhi