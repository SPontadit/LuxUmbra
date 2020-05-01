#include "rhi\RHI.h"

#include <array>
#include <map>
#include <random>

#include "glm\glm.hpp"
#include "glm\gtx\transform.hpp"

#include "utility\Utility.h"

namespace lux::rhi
{
	using sortedMeshNodesIterator = std::map<std::string, std::vector<scene::MeshNode*>>::iterator;
	using sortedMeshNodesConstIterator = std::map<std::string, std::vector<scene::MeshNode*>>::const_iterator;
	
	ForwardRenderer::ForwardRenderer() noexcept
		: rtImageFormat(VK_FORMAT_R32G32B32A32_SFLOAT), rtRenderPass(VK_NULL_HANDLE), rtFrameBuffers(0), descriptorPool(VK_NULL_HANDLE),
		blitRenderPass(VK_NULL_HANDLE), blitFrameBuffers(0), blitGraphicsPipeline(), blitGraphicsPipelineCI(), blitDescriptorSets(0),
		ssaoRenderPass(VK_NULL_HANDLE), ssaoFrameBuffers(0), ssaoColorAttachments(0),
		rtGraphicsPipeline(), rtCutoutGraphicsPipeline(), rtTransparentBackGraphicsPipeline(), rtTransparentFrontGraphicsPipeline(),
		rtGraphicsPipelineCI(), rtCutoutGraphicsPipelineCI(), rtTransparentBackGraphicsPipelineCI(), rtTransparentFrontGraphicsPipelineCI(),
		rtViewDescriptorSets(0), rtModelDescriptorSets(0), rtColorAttachmentImages(0), rtColorAttachmentImageMemories(0), rtColorAttachmentImageViews(0),
		rtDepthAttachmentImage(VK_NULL_HANDLE), rtDepthAttachmentMemory(VK_NULL_HANDLE), rtDepthAttachmentImageView(VK_NULL_HANDLE),
		envMapGraphicsPipeline(), envMapGraphicsPipelineCI(), envMapViewDescriptorSets(0), modelConstant(), viewProjUniformBuffers(0),
		sampler(VK_NULL_HANDLE), cubemapSampler(VK_NULL_HANDLE), irradianceSampler(VK_NULL_HANDLE), prefilteredSampler(VK_NULL_HANDLE)
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

		VkAttachmentDescription rtIndirectColorAttachment = {};
		rtIndirectColorAttachment.format = forward.rtImageFormat;
		rtIndirectColorAttachment.samples = msaaSamples;
		rtIndirectColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rtIndirectColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtIndirectColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtIndirectColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtIndirectColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtIndirectColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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

		VkAttachmentDescription rtResolveIndirectColorAttachment = {};
		rtResolveIndirectColorAttachment.format = forward.rtImageFormat;
		rtResolveIndirectColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		rtResolveIndirectColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveIndirectColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rtResolveIndirectColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rtResolveIndirectColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rtResolveIndirectColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		rtResolveIndirectColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference rtColorAttachmentRef = {};
		rtColorAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT;
		rtColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtPositionAttachmentRef = {};
		rtPositionAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_POSITION_ATTACHMENT_BIND_POINT;
		rtPositionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtNormalAttachmentRef = {};
		rtNormalAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_NORMAL_ATTACHMENT_BIND_POINT;
		rtNormalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtIndirectColorAttachmentRef = {};
		rtIndirectColorAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_INDIRECT_COLOR_ATTACHMENT_BIND_POINT;
		rtIndirectColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtDepthAttachmentRef = {};
		rtDepthAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT;
		rtDepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtResolveColorAttachmentRef = {};
		rtResolveColorAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_RESOLVE_COLOR_ATTACHMENT_BIND_POINT;
		rtResolveColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtResolvePositionAttachmentRef = {};
		rtResolvePositionAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_RESOLVE_POSITION_ATTACHMENT_BIND_POINT;
		rtResolvePositionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtResolveNormalAttachmentRef = {};
		rtResolveNormalAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_RESOLVE_NORMAL_ATTACHMENT_BIND_POINT;
		rtResolveNormalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rtResolveIndirectColorAttachmentRef = {};
		rtResolveIndirectColorAttachmentRef.attachment = ForwardRenderer::FORWARD_RT_RESOLVE_INDIRECT_COLOR_ATTACHMENT_BIND_POINT;
		rtResolveIndirectColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentReference, 4> colorAttachments = { rtColorAttachmentRef, rtPositionAttachmentRef, rtNormalAttachmentRef, rtIndirectColorAttachmentRef };
		std::array<VkAttachmentReference, 4> resolveAttachments = { rtResolveColorAttachmentRef, rtResolvePositionAttachmentRef, rtResolveNormalAttachmentRef, rtResolveIndirectColorAttachmentRef };

		VkSubpassDescription renderToTargetSubpass = {};
		renderToTargetSubpass.colorAttachmentCount = TO_UINT32_T(colorAttachments.size());
		renderToTargetSubpass.pColorAttachments = colorAttachments.data();
		renderToTargetSubpass.pDepthStencilAttachment = &rtDepthAttachmentRef;
		renderToTargetSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		renderToTargetSubpass.pResolveAttachments = resolveAttachments.data();

		std::array<VkAttachmentDescription, TO_SIZE_T(ForwardRenderer::FORWARD_RT_ATTACHMENT_BIND_POINT_COUNT)> attachments{
			rtColorAttachment,
			rtDepthAttachment,
			rtPositionAttachment,
			rtNormalAttachment,
			rtIndirectColorAttachment,
			rtResolveColorAttachment,
			rtResolvePositionAttachment,
			rtResolveNormalAttachment,
			rtResolveIndirectColorAttachment
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

		VkAttachmentDescription SSAOAttachment = {};
		SSAOAttachment.format = VK_FORMAT_R8_UNORM;
		SSAOAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		SSAOAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		SSAOAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		SSAOAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		SSAOAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		SSAOAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		SSAOAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference SSAOAttachmentRef = {};
		SSAOAttachmentRef.attachment = 0;
		SSAOAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkSubpassDescription SSAOSubpass = {};
		SSAOSubpass.colorAttachmentCount = 1;
		SSAOSubpass.pColorAttachments = &SSAOAttachmentRef;
		SSAOSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		std::array<VkSubpassDependency, 2> SSAOSubpassDependencies;
		SSAOSubpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		SSAOSubpassDependencies[0].dstSubpass = 0;
		SSAOSubpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		SSAOSubpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		SSAOSubpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		SSAOSubpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		SSAOSubpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		SSAOSubpassDependencies[1].srcSubpass = 0;
		SSAOSubpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		SSAOSubpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		SSAOSubpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		SSAOSubpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		SSAOSubpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		SSAOSubpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;



		VkRenderPassCreateInfo SSAORenderPassCI = {};
		SSAORenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		SSAORenderPassCI.attachmentCount = 1;
		SSAORenderPassCI.pAttachments = &SSAOAttachment;
		SSAORenderPassCI.subpassCount = 1;
		SSAORenderPassCI.pSubpasses = &SSAOSubpass;
		SSAORenderPassCI.dependencyCount = TO_UINT32_T(SSAOSubpassDependencies.size());
		SSAORenderPassCI.pDependencies = SSAOSubpassDependencies.data();

		CHECK_VK(vkCreateRenderPass(device, &SSAORenderPassCI, nullptr, &forward.ssaoRenderPass));


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


		ImageCreateInfo rtResolveColorAttachmentCI = {};
		rtResolveColorAttachmentCI.format = forward.rtImageFormat;
		rtResolveColorAttachmentCI.width = swapchainExtent.width;
		rtResolveColorAttachmentCI.height = swapchainExtent.height;
		rtResolveColorAttachmentCI.arrayLayers = 1;
		rtResolveColorAttachmentCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		rtResolveColorAttachmentCI.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		rtResolveColorAttachmentCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		rtResolveColorAttachmentCI.subresourceRangeLayerCount = 1;
		rtResolveColorAttachmentCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(rtResolveColorAttachmentCI, forward.rtResolveColorAttachment);


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

		ImageCreateInfo rtResolvePositionImageCI = {};
		rtResolvePositionImageCI.format = forward.rtImageFormat;
		rtResolvePositionImageCI.width = swapchainExtent.width;
		rtResolvePositionImageCI.height = swapchainExtent.height;
		rtResolvePositionImageCI.arrayLayers = 1;
		rtResolvePositionImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		rtResolvePositionImageCI.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		rtResolvePositionImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		rtResolvePositionImageCI.subresourceRangeLayerCount = 1;
		rtResolvePositionImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(rtResolvePositionImageCI, forward.rtResolvePositionMap);


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

		ImageCreateInfo rtResolveNormalImageCI = {};
		rtResolveNormalImageCI.format = forward.rtImageFormat;
		rtResolveNormalImageCI.width = swapchainExtent.width;
		rtResolveNormalImageCI.height = swapchainExtent.height;
		rtResolveNormalImageCI.arrayLayers = 1;
		rtResolveNormalImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		rtResolveNormalImageCI.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		rtResolveNormalImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		rtResolveNormalImageCI.subresourceRangeLayerCount = 1;
		rtResolveNormalImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(rtResolveNormalImageCI, forward.rtResolveNormalMap);


		// Indirect Color Map
		ImageCreateInfo rtIndirectColorImageCI = {};
		rtIndirectColorImageCI.format = forward.rtImageFormat;
		rtIndirectColorImageCI.width = swapchainExtent.width;
		rtIndirectColorImageCI.height = swapchainExtent.height;
		rtIndirectColorImageCI.arrayLayers = 1;
		rtIndirectColorImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		rtIndirectColorImageCI.sampleCount = msaaSamples;
		rtIndirectColorImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		rtIndirectColorImageCI.subresourceRangeLayerCount = 1;
		rtIndirectColorImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(rtNormalImageCI, forward.rtIndirectColorMap);

		ImageCreateInfo rtResolveIndirectColorImageCI = {};
		rtResolveIndirectColorImageCI.format = forward.rtImageFormat;
		rtResolveIndirectColorImageCI.width = swapchainExtent.width;
		rtResolveIndirectColorImageCI.height = swapchainExtent.height;
		rtResolveIndirectColorImageCI.arrayLayers = 1;
		rtResolveIndirectColorImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		rtResolveIndirectColorImageCI.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		rtResolveIndirectColorImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		rtResolveIndirectColorImageCI.subresourceRangeLayerCount = 1;
		rtResolveIndirectColorImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(rtResolveIndirectColorImageCI, forward.rtResolveIndirectColorMap);

		
		// SSAO Image
		ImageCreateInfo ssaoImageCI = {};
		ssaoImageCI.format = VK_FORMAT_R8_UNORM;
		ssaoImageCI.width = swapchainExtent.width;
		ssaoImageCI.height = swapchainExtent.height;
		ssaoImageCI.arrayLayers = 1;
		ssaoImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		ssaoImageCI.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		ssaoImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ssaoImageCI.subresourceRangeLayerCount = 1;
		ssaoImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;


		forward.ssaoColorAttachments.resize(TO_SIZE_T(swapchainImageCount));
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			CreateImage(ssaoImageCI, forward.ssaoColorAttachments[i]);
		}

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

		VkFramebufferCreateInfo SSAOFramebufferCI = {};
		SSAOFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		SSAOFramebufferCI.renderPass = forward.ssaoRenderPass;
		SSAOFramebufferCI.width = swapchainExtent.width;
		SSAOFramebufferCI.height = swapchainExtent.height;
		SSAOFramebufferCI.layers = 1;
		SSAOFramebufferCI.attachmentCount = 1;

		forward.rtFrameBuffers.resize(TO_SIZE_T(swapchainImageCount));
		forward.blitFrameBuffers.resize(TO_SIZE_T(swapchainImageCount));
		forward.ssaoFrameBuffers.resize(TO_SIZE_T(swapchainImageCount));

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			blitFramebufferCI.pAttachments = &swapchainImageViews[i];
			SSAOFramebufferCI.pAttachments = &forward.ssaoColorAttachments[i].imageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT)] = forward.rtColorAttachmentImageViews[i];
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT)] = forward.rtDepthAttachmentImageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_POSITION_ATTACHMENT_BIND_POINT)] = forward.rtPositionMap.imageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_NORMAL_ATTACHMENT_BIND_POINT)] = forward.rtNormalMap.imageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_INDIRECT_COLOR_ATTACHMENT_BIND_POINT)] = forward.rtIndirectColorMap.imageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_RESOLVE_COLOR_ATTACHMENT_BIND_POINT)] = forward.rtResolveColorAttachment.imageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_RESOLVE_POSITION_ATTACHMENT_BIND_POINT)] = forward.rtResolvePositionMap.imageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_RESOLVE_NORMAL_ATTACHMENT_BIND_POINT)] = forward.rtResolveNormalMap.imageView;
			attachments[TO_SIZE_T(ForwardRenderer::FORWARD_RT_RESOLVE_INDIRECT_COLOR_ATTACHMENT_BIND_POINT)] = forward.rtResolveIndirectColorMap.imageView;

			CHECK_VK(vkCreateFramebuffer(device, &rtFramebufferCI, nullptr, &forward.rtFrameBuffers[i]));
			CHECK_VK(vkCreateFramebuffer(device, &blitFramebufferCI, nullptr, &forward.blitFrameBuffers[i]));
			CHECK_VK(vkCreateFramebuffer(device, &SSAOFramebufferCI, nullptr, &forward.ssaoFrameBuffers[i]));
		}

	}

	void RHI::InitForwardDescriptorPool() noexcept
	{
		VkDescriptorPoolSize blitSamplersDescriptorPoolSize = {};
		blitSamplersDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		blitSamplersDescriptorPoolSize.descriptorCount = swapchainImageCount * 3;

		VkDescriptorPoolSize SSAOSamplersDescriptorPoolSize = {};
		SSAOSamplersDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		SSAOSamplersDescriptorPoolSize.descriptorCount = swapchainImageCount * 3;

		VkDescriptorPoolSize SSAOKernelDescriptorPoolSize = {};
		SSAOKernelDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		SSAOKernelDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize rtViewProjUniformDescriptorPoolSize = {};
		rtViewProjUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		rtViewProjUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize directionalLightUniformDescriptorPoolSize = {};
		directionalLightUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		directionalLightUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize pointLightUniformDescriptorPoolSize = {};
		pointLightUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pointLightUniformDescriptorPoolSize.descriptorCount = swapchainImageCount;

		VkDescriptorPoolSize directionalLightShadowMapsDescriptorPoolSize = {};
		directionalLightShadowMapsDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		directionalLightShadowMapsDescriptorPoolSize.descriptorCount = swapchainImageCount * DIRECTIONAL_LIGHT_MAX_COUNT;

		VkDescriptorPoolSize pointLightShadowMapsDescriptorPoolSize = {};
		pointLightShadowMapsDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pointLightShadowMapsDescriptorPoolSize.descriptorCount = swapchainImageCount * POINT_LIGHT_MAX_COUNT;

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

		std::array<VkDescriptorPoolSize, 13> descriptorPoolSizes = 
		{ 
			blitSamplersDescriptorPoolSize,
			SSAOSamplersDescriptorPoolSize,
			SSAOKernelDescriptorPoolSize,
			rtViewProjUniformDescriptorPoolSize, 
			directionalLightUniformDescriptorPoolSize,
			pointLightUniformDescriptorPoolSize,
			directionalLightShadowMapsDescriptorPoolSize,
			pointLightShadowMapsDescriptorPoolSize,
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

		VkDescriptorSetLayoutBinding SSAOMapDescriptorSetLayoutBinding = {};
		SSAOMapDescriptorSetLayoutBinding.binding = 1;
		SSAOMapDescriptorSetLayoutBinding.descriptorCount = 1;
		SSAOMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		SSAOMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding indirectColorMapDescriptorSetLayoutBinding = {};
		indirectColorMapDescriptorSetLayoutBinding.binding = 2;
		indirectColorMapDescriptorSetLayoutBinding.descriptorCount = 1;
		indirectColorMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		indirectColorMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPushConstantRange blitPostProcessParameterPushConstantRange = {};
		blitPostProcessParameterPushConstantRange.offset = 0;
		blitPostProcessParameterPushConstantRange.size = sizeof(PostProcessParameters);
		blitPostProcessParameterPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		forward.postProcessParameters.inverseScreenSize = { TO_FLOAT(1.0 / swapchainExtent.width), TO_FLOAT(1.0 / swapchainExtent.height) };

		forward.blitGraphicsPipelineCI = {};
		forward.blitGraphicsPipelineCI.renderPass = forward.blitRenderPass;
		forward.blitGraphicsPipelineCI.subpassIndex = 0;
		forward.blitGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/blit/blit.vert.spv";
		forward.blitGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/blit/blit.frag.spv";
		forward.blitGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/blitGraphics.bin";
		forward.blitGraphicsPipelineCI.vertexLayout = lux::VertexLayout::NO_VERTEX_LAYOUT;
		forward.blitGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		forward.blitGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		forward.blitGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		forward.blitGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		forward.blitGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		forward.blitGraphicsPipelineCI.disableMSAA = true;
		forward.blitGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		forward.blitGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		forward.blitGraphicsPipelineCI.enableDepthBias = VK_FALSE;
		forward.blitGraphicsPipelineCI.depthBiasConstantFactor = 0.f;
		forward.blitGraphicsPipelineCI.depthBiasSlopeFactor = 0.f;
		forward.blitGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS;

		forward.blitGraphicsPipelineCI.viewDescriptorSetLayoutBindings =
		{
			blitDescriptorSetLayoutBinding,
			SSAOMapDescriptorSetLayoutBinding,
			indirectColorMapDescriptorSetLayoutBinding
		};

		forward.blitGraphicsPipelineCI.pushConstants = { blitPostProcessParameterPushConstantRange };

		CreateGraphicsPipeline(forward.blitGraphicsPipelineCI, forward.blitGraphicsPipeline);


		// SSAO
		VkDescriptorSetLayoutBinding positionMapDescriptorSetLayoutBinding = {};
		positionMapDescriptorSetLayoutBinding.binding = 0;
		positionMapDescriptorSetLayoutBinding.descriptorCount = 1;
		positionMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		positionMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding normalMapDescriptorSetLayoutBinding = {};
		normalMapDescriptorSetLayoutBinding.binding = 1;
		normalMapDescriptorSetLayoutBinding.descriptorCount = 1;
		normalMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding SSAONoiseDescriptorSetLayoutBinding = {};
		SSAONoiseDescriptorSetLayoutBinding.binding = 2;
		SSAONoiseDescriptorSetLayoutBinding.descriptorCount = 1;
		SSAONoiseDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		SSAONoiseDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding SSAOKernelsDescriptorSetLayoutBinding = {};
		SSAOKernelsDescriptorSetLayoutBinding.binding = 3;
		SSAOKernelsDescriptorSetLayoutBinding.descriptorCount = 1;
		SSAOKernelsDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		SSAOKernelsDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		
		VkPushConstantRange SSAOPushConstantRange = {};
		SSAOPushConstantRange.offset = 0;
		SSAOPushConstantRange.size = sizeof(SSAOParameters);
		SSAOPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		GraphicsPipelineCreateInfo ssaoGraphicsPipelineCI = {};
		ssaoGraphicsPipelineCI.renderPass = forward.ssaoRenderPass;
		ssaoGraphicsPipelineCI.subpassIndex = 0;
		ssaoGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/SSAO/SSAO.vert.spv";
		ssaoGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/SSAO/SSAO.frag.spv";
		ssaoGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/SSAO.bin";
		ssaoGraphicsPipelineCI.vertexLayout = lux::VertexLayout::NO_VERTEX_LAYOUT;
		ssaoGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		ssaoGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		ssaoGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		ssaoGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		ssaoGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		ssaoGraphicsPipelineCI.disableMSAA = true;
		ssaoGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		ssaoGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		ssaoGraphicsPipelineCI.enableDepthBias = VK_FALSE;
		ssaoGraphicsPipelineCI.depthBiasConstantFactor = 0.f;
		ssaoGraphicsPipelineCI.depthBiasSlopeFactor = 0.f;
		ssaoGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS;
		ssaoGraphicsPipelineCI.viewDescriptorSetLayoutBindings =
		{
			positionMapDescriptorSetLayoutBinding,
			normalMapDescriptorSetLayoutBinding,
			SSAOKernelsDescriptorSetLayoutBinding,
			SSAONoiseDescriptorSetLayoutBinding
		};
		ssaoGraphicsPipelineCI.pushConstants = { SSAOPushConstantRange };


		CreateGraphicsPipeline(ssaoGraphicsPipelineCI, forward.ssaoGraphicsPipeline);


		// Render Target Graphics Pipeline
		
		// View Layout
		VkDescriptorSetLayoutBinding rtViewProjDescriptorSetLayoutBinding = {};
		rtViewProjDescriptorSetLayoutBinding.binding = 0;
		rtViewProjDescriptorSetLayoutBinding.descriptorCount = 1;
		rtViewProjDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		rtViewProjDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding directionalLightUniformDescriptorSetLayoutBinding = {};
		directionalLightUniformDescriptorSetLayoutBinding.binding = 1;
		directionalLightUniformDescriptorSetLayoutBinding.descriptorCount = 1;
		directionalLightUniformDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		directionalLightUniformDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding pointLightUniformDescriptorSetLayoutBinding = {};
		pointLightUniformDescriptorSetLayoutBinding.binding = 2;
		pointLightUniformDescriptorSetLayoutBinding.descriptorCount = 1;
		pointLightUniformDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pointLightUniformDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding directionalLightShadowMapsDescriptorSetLayoutBinding = {};
		directionalLightShadowMapsDescriptorSetLayoutBinding.binding = 3;
		directionalLightShadowMapsDescriptorSetLayoutBinding.descriptorCount = DIRECTIONAL_LIGHT_MAX_COUNT;
		directionalLightShadowMapsDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		directionalLightShadowMapsDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding pointLightShadowMapsDescriptorSetLayoutBinding = {};
		pointLightShadowMapsDescriptorSetLayoutBinding.binding = 4;
		pointLightShadowMapsDescriptorSetLayoutBinding.descriptorCount = POINT_LIGHT_MAX_COUNT;
		pointLightShadowMapsDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pointLightShadowMapsDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding irradianceMapDescriptorSetLayoutBinding = {};
		irradianceMapDescriptorSetLayoutBinding.binding = 5;
		irradianceMapDescriptorSetLayoutBinding.descriptorCount = 1;
		irradianceMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		irradianceMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding prefilteredMapDescriptorSetLayoutBinding = {};
		prefilteredMapDescriptorSetLayoutBinding.binding = 6;
		prefilteredMapDescriptorSetLayoutBinding.descriptorCount = 1;
		prefilteredMapDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		prefilteredMapDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding BRDFLutDescriptorSetLayoutBinding = {};
		BRDFLutDescriptorSetLayoutBinding.binding = 7;
		BRDFLutDescriptorSetLayoutBinding.descriptorCount = 1;
		BRDFLutDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		BRDFLutDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


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
		lightCountPushConstantRange.size = sizeof(LightCountsPushConstant);
		lightCountPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


		// Graphics Pipeline

		forward.rtGraphicsPipelineCI = {};
		forward.rtGraphicsPipelineCI.renderPass = forward.rtRenderPass;
		forward.rtGraphicsPipelineCI.subpassIndex = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		forward.rtGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLight.vert.spv";
		forward.rtGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLight.frag.spv";
		forward.rtGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtGraphics.bin";
		forward.rtGraphicsPipelineCI.vertexLayout = lux::VertexLayout::VERTEX_FULL_LAYOUT;
		forward.rtGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		forward.rtGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		forward.rtGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		forward.rtGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		forward.rtGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		forward.rtGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		forward.rtGraphicsPipelineCI.enableDepthWrite = VK_TRUE;
		forward.rtGraphicsPipelineCI.enableDepthBias = VK_FALSE;
		forward.rtGraphicsPipelineCI.depthBiasConstantFactor = 0.f;
		forward.rtGraphicsPipelineCI.depthBiasSlopeFactor = 0.f;
		forward.rtGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS;
		forward.rtGraphicsPipelineCI.colorBlendAttachmentStateCount = 4;
		
		forward.rtGraphicsPipelineCI.viewDescriptorSetLayoutBindings = 
		{ 
			rtViewProjDescriptorSetLayoutBinding, 
			directionalLightUniformDescriptorSetLayoutBinding,
			pointLightUniformDescriptorSetLayoutBinding,
			directionalLightShadowMapsDescriptorSetLayoutBinding,
			pointLightShadowMapsDescriptorSetLayoutBinding,
			irradianceMapDescriptorSetLayoutBinding, 
			prefilteredMapDescriptorSetLayoutBinding,
			BRDFLutDescriptorSetLayoutBinding,
		};

		forward.rtGraphicsPipelineCI.materialDescriptorSetLayoutBindings =
		{ 
			materialParametersDescriptorSetLayoutBinding, 
			materialAlbedoDescriptorSetLayoutBinding, 
			materialNormalDescriptorSetLayoutBinding,
			materialMetallicRoughnessDescriptorSetLayoutBinding,
			materialAmbientOcclusionDescriptorSetLayoutBinding,
		};

		forward.rtGraphicsPipelineCI.pushConstants = { rtModelPushConstantRange, lightCountPushConstantRange };

		CreateGraphicsPipeline(forward.rtGraphicsPipelineCI, forward.rtGraphicsPipeline);


		// Cutout Graphics Pipeline
		forward.rtCutoutGraphicsPipelineCI = forward.rtGraphicsPipelineCI;
		forward.rtCutoutGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLightCutout.frag.spv";
		forward.rtCutoutGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtCutoutGraphics.bin";
		forward.rtCutoutGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		forward.rtCutoutGraphicsPipelineCI.disableColorWriteMask = true;
		forward.rtCutoutGraphicsPipelineCI.enableBlend = true;

		CreateGraphicsPipeline(forward.rtCutoutGraphicsPipelineCI, forward.rtCutoutGraphicsPipeline);


		// Front Face Transparent Graphics Pipeline
		forward.rtTransparentFrontGraphicsPipelineCI = forward.rtCutoutGraphicsPipelineCI;
		forward.rtTransparentFrontGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/cameraSpaceLight/cameraSpaceLight.frag.spv";
		forward.rtTransparentFrontGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtTransparentFrontGraphics.bin";
		forward.rtTransparentFrontGraphicsPipelineCI.disableColorWriteMask = false;
		forward.rtTransparentFrontGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		forward.rtTransparentFrontGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;

		CreateGraphicsPipeline(forward.rtTransparentFrontGraphicsPipelineCI, forward.rtTransparentFrontGraphicsPipeline);


		// Back Face Transparent Graphics Pipeline
		forward.rtTransparentBackGraphicsPipelineCI = forward.rtTransparentFrontGraphicsPipelineCI;
		forward.rtTransparentBackGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_FRONT_BIT;
		forward.rtTransparentBackGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtTransparentBackGraphics.bin";

		CreateGraphicsPipeline(forward.rtTransparentBackGraphicsPipelineCI, forward.rtTransparentBackGraphicsPipeline);


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

		forward.envMapGraphicsPipelineCI = {};
		forward.envMapGraphicsPipelineCI.renderPass = forward.rtRenderPass;
		forward.envMapGraphicsPipelineCI.subpassIndex = ForwardRenderer::FORWARD_SUBPASS_RENDER_TO_TARGET;
		forward.envMapGraphicsPipelineCI.binaryVertexFilePath = "data/shaders/envMap/envMap.vert.spv";
		forward.envMapGraphicsPipelineCI.binaryFragmentFilePath = "data/shaders/envMap/envMap.frag.spv";
		forward.envMapGraphicsPipelineCI.cacheFilePath = "data/pipelineCache/rtEnvMapGraphics.bin";
		forward.envMapGraphicsPipelineCI.vertexLayout = lux::VertexLayout::VERTEX_POSITION_ONLY_LAYOUT;
		forward.envMapGraphicsPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		forward.envMapGraphicsPipelineCI.viewportWidth = TO_FLOAT(swapchainExtent.width);
		forward.envMapGraphicsPipelineCI.viewportHeight = TO_FLOAT(swapchainExtent.height);
		forward.envMapGraphicsPipelineCI.rasterizerCullMode = VK_CULL_MODE_NONE;
		forward.envMapGraphicsPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_CLOCKWISE;
		forward.envMapGraphicsPipelineCI.colorBlendAttachmentStateCount = 4;
		forward.envMapGraphicsPipelineCI.enableDepthTest = VK_TRUE;
		forward.envMapGraphicsPipelineCI.enableDepthWrite = VK_FALSE;
		forward.envMapGraphicsPipelineCI.enableDepthBias = VK_FALSE;
		forward.envMapGraphicsPipelineCI.depthBiasConstantFactor = 0.f;
		forward.envMapGraphicsPipelineCI.depthBiasSlopeFactor = 0.f;
		forward.envMapGraphicsPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		forward.envMapGraphicsPipelineCI.viewDescriptorSetLayoutBindings = { envMapViewProjDescriptorSetLayoutBinding, envMapSamplerDescriptorSetLayoutBinding};
	
		CreateGraphicsPipeline(forward.envMapGraphicsPipelineCI, forward.envMapGraphicsPipeline);
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


		samplerCI.magFilter = VK_FILTER_NEAREST;
		samplerCI.minFilter = VK_FILTER_NEAREST;
		samplerCI.compareOp = VK_COMPARE_OP_NEVER;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		CHECK_VK(vkCreateSampler(device, &samplerCI, nullptr, &forward.SSAONoiseSampler));


		samplerCI.magFilter = VK_FILTER_LINEAR;
		samplerCI.minFilter = VK_FILTER_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		samplerCI.maxLod = TO_FLOAT(floor(log2(CUBEMAP_TEXTURE_SIZE))) + 1.0f;
		samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;

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
		blitDescriptorImageInfo.imageView = forward.rtResolveColorAttachment.imageView;

		VkWriteDescriptorSet writeBlitDescriptorSet = {};
		writeBlitDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeBlitDescriptorSet.descriptorCount = 1;
		writeBlitDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeBlitDescriptorSet.dstBinding = 0;
		writeBlitDescriptorSet.pImageInfo = &blitDescriptorImageInfo;

		VkDescriptorImageInfo SSAOMapDescriptorImageInfo = {};
		SSAOMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		SSAOMapDescriptorImageInfo.sampler = forward.sampler;

		VkWriteDescriptorSet writeSSAOMapDescriptorSet = {};
		writeSSAOMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSSAOMapDescriptorSet.descriptorCount = 1;
		writeSSAOMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSSAOMapDescriptorSet.dstBinding = 1;
		writeSSAOMapDescriptorSet.pImageInfo = &SSAOMapDescriptorImageInfo;

		VkDescriptorImageInfo indirectColorMapDescriptorImageInfo = {};
		indirectColorMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		indirectColorMapDescriptorImageInfo.sampler = forward.sampler;
		indirectColorMapDescriptorImageInfo.imageView = forward.rtResolveIndirectColorMap.imageView;

		VkWriteDescriptorSet writeIndirectColorMapDescriptorSet = {};
		writeIndirectColorMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeIndirectColorMapDescriptorSet.descriptorCount = 1;
		writeIndirectColorMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeIndirectColorMapDescriptorSet.dstBinding = 2;
		writeIndirectColorMapDescriptorSet.pImageInfo = &indirectColorMapDescriptorImageInfo;
		
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			SSAOMapDescriptorImageInfo.imageView = forward.ssaoColorAttachments[i].imageView;

			writeBlitDescriptorSet.dstSet = forward.blitDescriptorSets[i];
			writeSSAOMapDescriptorSet.dstSet = forward.blitDescriptorSets[i];
			writeIndirectColorMapDescriptorSet.dstSet = forward.blitDescriptorSets[i];

			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets = 
			{
				writeBlitDescriptorSet,
				writeSSAOMapDescriptorSet,
				writeIndirectColorMapDescriptorSet
			};

			vkUpdateDescriptorSets(device, TO_UINT32_T(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}


		// Allocate SSAO Descriptor Set
		std::vector<VkDescriptorSetLayout> SSAODescriptorSetLayout(swapchainImageCount, forward.ssaoGraphicsPipeline.viewDescriptorSetLayout);
		VkDescriptorSetAllocateInfo ssaoDescriptorSetAI = {};
		ssaoDescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ssaoDescriptorSetAI.descriptorPool = forward.descriptorPool;
		ssaoDescriptorSetAI.descriptorSetCount = swapchainImageCount;
		ssaoDescriptorSetAI.pSetLayouts = SSAODescriptorSetLayout.data();

		forward.ssaoDescriptorSets.resize(TO_SIZE_T(swapchainImageCount));
		CHECK_VK(vkAllocateDescriptorSets(device, &ssaoDescriptorSetAI, forward.ssaoDescriptorSets.data()));


		VkDescriptorImageInfo positionMapDescriptorImageInfo = {};
		positionMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		positionMapDescriptorImageInfo.sampler = forward.sampler;
		positionMapDescriptorImageInfo.imageView = forward.rtResolvePositionMap.imageView;

		VkWriteDescriptorSet writePositionMapDescriptorSet = {};
		writePositionMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writePositionMapDescriptorSet.descriptorCount = 1;
		writePositionMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writePositionMapDescriptorSet.dstBinding = 0;
		writePositionMapDescriptorSet.pImageInfo = &positionMapDescriptorImageInfo;


		VkDescriptorImageInfo normalMapDescriptorImageInfo = {};
		normalMapDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalMapDescriptorImageInfo.sampler = forward.sampler;
		normalMapDescriptorImageInfo.imageView = forward.rtResolveNormalMap.imageView;

		VkWriteDescriptorSet writeNormalMapDescriptorSet = {};
		writeNormalMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeNormalMapDescriptorSet.descriptorCount = 1;
		writeNormalMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeNormalMapDescriptorSet.dstBinding = 1;
		writeNormalMapDescriptorSet.pImageInfo = &normalMapDescriptorImageInfo;

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			writePositionMapDescriptorSet.dstSet = forward.ssaoDescriptorSets[i];
			writeNormalMapDescriptorSet.dstSet = forward.ssaoDescriptorSets[i];

			std::array<VkWriteDescriptorSet, 2> writeDescriptorSets =
			{
				writePositionMapDescriptorSet,
				writeNormalMapDescriptorSet
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

		// Light UBOs
		VkDescriptorBufferInfo directionalLightDescriptorBufferInfo = {};
		directionalLightDescriptorBufferInfo.offset = 0;
		directionalLightDescriptorBufferInfo.range = sizeof(DirectionalLightBuffer) * DIRECTIONAL_LIGHT_MAX_COUNT;

		VkWriteDescriptorSet writeDirectionalLightDescriptorSet = {};
		writeDirectionalLightDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDirectionalLightDescriptorSet.descriptorCount = 1;
		writeDirectionalLightDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDirectionalLightDescriptorSet.dstBinding = 1;
		writeDirectionalLightDescriptorSet.dstArrayElement = 0;
		writeDirectionalLightDescriptorSet.pBufferInfo = &directionalLightDescriptorBufferInfo;

		VkDescriptorBufferInfo pointLightDescriptorBufferInfo = {};
		pointLightDescriptorBufferInfo.offset = 0;
		pointLightDescriptorBufferInfo.range = sizeof(PointLightBuffer) * POINT_LIGHT_MAX_COUNT;

		VkWriteDescriptorSet writePointLightDescriptorSet = {};
		writePointLightDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writePointLightDescriptorSet.descriptorCount = 1;
		writePointLightDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writePointLightDescriptorSet.dstBinding = 2;
		writePointLightDescriptorSet.dstArrayElement = 0;
		writePointLightDescriptorSet.pBufferInfo = &pointLightDescriptorBufferInfo;

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			rtViewProjDescriptorBufferInfo.buffer = forward.viewProjUniformBuffers[i].buffer;
			rtWriteViewProjDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			directionalLightDescriptorBufferInfo.buffer = directionalLightUniformBuffers[i].buffer;
			writeDirectionalLightDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			pointLightDescriptorBufferInfo.buffer = pointLightUniformBuffers[i].buffer;
			writePointLightDescriptorSet.dstSet = forward.rtViewDescriptorSets[i];

			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets = {
				rtWriteViewProjDescriptorSet,
				writeDirectionalLightDescriptorSet,
				writePointLightDescriptorSet
			};

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

	void RHI::GenerateSSAOKernels() noexcept
	{
		uint32_t ssaoKernelSize = 32;
		uint32_t ssaoNoiseDimension = 4;
		float ssaoRadius = 0.5f;

		std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
		std::default_random_engine generator;
		std::vector<glm::vec4> ssaoKernel(ssaoKernelSize);
		std::vector<glm::vec4> ssaoNoise(ssaoNoiseDimension * ssaoNoiseDimension);

		for (size_t i = 0; i < ssaoKernelSize; i++)
		{
			float x = randomFloats(generator) * 2.0f - 1.0f;
			float y = randomFloats(generator) * 2.0f - 1.0f;
			float z = randomFloats(generator);
			
			glm::vec3 sample(x, y, z);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = TO_FLOAT(i) / TO_FLOAT(ssaoKernelSize);
			scale = utility::Lerp(0.1f, 1.0f, scale * scale);

			ssaoKernel[i] = glm::vec4(sample * scale, 0.0);
		}


		BufferCreateInfo SSAOKernelBufferCI = {};
		SSAOKernelBufferCI.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		SSAOKernelBufferCI.size = ssaoKernel.size() * sizeof(glm::vec4);
		SSAOKernelBufferCI.data = ssaoKernel.data();
		SSAOKernelBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SSAOKernelBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		CreateBuffer(SSAOKernelBufferCI, forward.SSAOKernelsUniformBuffer);


		for (size_t i = 0; i < ssaoNoise.size(); i++)
		{
			float x = randomFloats(generator) * 2.0f - 1.0f;
			float y = randomFloats(generator) * 2.0f - 1.0f;
			ssaoNoise[i] = glm::vec4(x, y, 0.0f, 0.0f);
		}

		ImageCreateInfo ssaoNoiseImageCI = {};
		ssaoNoiseImageCI.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		ssaoNoiseImageCI.width = ssaoNoiseDimension;
		ssaoNoiseImageCI.height = ssaoNoiseDimension;
		ssaoNoiseImageCI.arrayLayers = 1;
		ssaoNoiseImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		ssaoNoiseImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ssaoNoiseImageCI.subresourceRangeLayerCount = 1;
		ssaoNoiseImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImageFromBuffer(ssaoNoiseImageCI, ssaoNoise.data(), TO_UINT32_T(ssaoNoise.size()) * sizeof(glm::vec4), forward.SSAONoiseImage);


		VkDescriptorBufferInfo SSAOKernelsDescriptorBufferInfo = {};
		SSAOKernelsDescriptorBufferInfo.offset = 0;
		SSAOKernelsDescriptorBufferInfo.range = ssaoKernel.size() * sizeof(glm::vec4);
		SSAOKernelsDescriptorBufferInfo.buffer = forward.SSAOKernelsUniformBuffer.buffer;

		VkWriteDescriptorSet writeSSAOKernelsDescriptorSet = {};
		writeSSAOKernelsDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSSAOKernelsDescriptorSet.descriptorCount = 1;
		writeSSAOKernelsDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeSSAOKernelsDescriptorSet.dstBinding = 3;
		writeSSAOKernelsDescriptorSet.pBufferInfo = &SSAOKernelsDescriptorBufferInfo;


		VkDescriptorImageInfo SSAONoiseDescriptorImageInfo = {};
		SSAONoiseDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		SSAONoiseDescriptorImageInfo.sampler = forward.SSAONoiseSampler;
		SSAONoiseDescriptorImageInfo.imageView = forward.SSAONoiseImage.imageView;

		VkWriteDescriptorSet writeSSAONoiseDescriptorSet = {};
		writeSSAONoiseDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSSAONoiseDescriptorSet.descriptorCount = 1;
		writeSSAONoiseDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSSAONoiseDescriptorSet.dstBinding = 2;
		writeSSAONoiseDescriptorSet.pImageInfo = &SSAONoiseDescriptorImageInfo;


		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			writeSSAOKernelsDescriptorSet.dstSet = forward.ssaoDescriptorSets[i];
			writeSSAONoiseDescriptorSet.dstSet = forward.ssaoDescriptorSets[i];

			std::array<VkWriteDescriptorSet, 2> writeDescriptorSets = 
			{
				writeSSAOKernelsDescriptorSet,
				writeSSAONoiseDescriptorSet
			};

			vkUpdateDescriptorSets(device, TO_UINT32_T(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void RHI::UpdateForwardUniformBuffers(const scene::CameraNode* camera, const std::vector<resource::Material*>& materials) noexcept
	{
		// Camera View & Proj

		forward.rtViewProjUniform.view = camera->GetViewTransform();
		forward.rtViewProjUniform.projection = camera->GetPerspectiveProjectionTransform();
		forward.rtViewProjUniform.nearFarPlane = glm::vec2(camera->GetNearDistance(), camera->GetFarDistance());

		UpdateBuffer(forward.viewProjUniformBuffers[currentFrame], &forward.rtViewProjUniform);

		// Materials

		for (size_t i = 0; i < materials.size(); i++)
		{
			UpdateBuffer(materials[i]->buffer[currentFrame], &materials[i]->parameter);
		}
	}

	void RHI::RenderForward(VkCommandBuffer commandBuffer, int imageIndex, const scene::CameraNode* camera, const std::vector<scene::MeshNode*>& meshes, const std::vector<scene::LightNode*>& lights) noexcept
	{

		VkDeviceSize vertexBufferOffsets[] = { 0 };

		VkClearColorValue clearColor{ 0.5f, 0.5703125f, 0.6171875f, 1.0F };

		
		std::array<VkClearValue, 5> clearValues = {};
		clearValues[ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT].color = clearColor;
		clearValues[ForwardRenderer::FORWARD_RT_POSITION_ATTACHMENT_BIND_POINT].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[ForwardRenderer::FORWARD_RT_NORMAL_ATTACHMENT_BIND_POINT].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[ForwardRenderer::FORWARD_RT_INDIRECT_COLOR_ATTACHMENT_BIND_POINT].color = { 0.0f, 0.0f, 0.0f, 0.0f };
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

		UpdateForwardUniformBuffers(camera, materials);

		// Render Target Subpass
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtGraphicsPipeline.pipelineLayout, ForwardRenderer::FORWARD_VIEW_DESCRIPTOR_SET_LAYOUT, 1, &forward.rtViewDescriptorSets[currentFrame], 0, nullptr);

		vkCmdPushConstants(commandBuffer, forward.rtGraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(RtModelConstant), sizeof(LightCountsPushConstant), &lightCountsPushConstant);

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

				//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.rtCutoutGraphicsPipeline.pipeline);
				//vkCmdBindVertexBuffers(commandBuffer, 0, 1, &currentMesh.vertexBuffer.buffer, vertexBufferOffsets);
				//vkCmdBindIndexBuffer(commandBuffer, currentMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				//vkCmdDrawIndexed(commandBuffer, currentMesh.indexCount, 1, 0, 0, 0);

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

	void RHI::RenderPostProcess(VkCommandBuffer commandBuffer, int imageIndex, const scene::CameraNode* camera) noexcept
	{
		VkDeviceSize vertexBufferOffsets[] = { 0 };

		VkClearColorValue clearColor{ 1.0f, 1.0f, 1.0f, 1.0F };

		std::array<VkClearValue, 1> clearValues = {};
		clearValues[ForwardRenderer::FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT].color = clearColor;


		// Begin SSAO Render Pass
		VkRenderPassBeginInfo SSAOrenderPassBI = {};
		SSAOrenderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		SSAOrenderPassBI.renderPass = forward.ssaoRenderPass;
		SSAOrenderPassBI.framebuffer = forward.ssaoFrameBuffers[imageIndex];
		SSAOrenderPassBI.renderArea.extent = swapchainExtent;
		SSAOrenderPassBI.clearValueCount = TO_UINT32_T(clearValues.size());
		SSAOrenderPassBI.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &SSAOrenderPassBI, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.ssaoGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.ssaoGraphicsPipeline.pipelineLayout, 0, 1, &forward.ssaoDescriptorSets[currentFrame], 0, nullptr);

		forward.ssaoParameters.proj = camera->GetPerspectiveProjectionTransform();
		vkCmdPushConstants(commandBuffer, forward.ssaoGraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SSAOParameters), &forward.ssaoParameters);

		vkCmdDraw(commandBuffer, 4, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);


		// Begin Blit Render Pass
		VkRenderPassBeginInfo blitRenderPassBI = {};
		blitRenderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		blitRenderPassBI.renderPass = forward.blitRenderPass;
		blitRenderPassBI.framebuffer = forward.blitFrameBuffers[imageIndex];
		blitRenderPassBI.renderArea.extent = swapchainExtent;
		blitRenderPassBI.clearValueCount = TO_UINT32_T(clearValues.size());
		blitRenderPassBI.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &blitRenderPassBI, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.blitGraphicsPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, forward.blitGraphicsPipeline.pipelineLayout, 0, 1, &forward.blitDescriptorSets[currentFrame], 0, nullptr);

		vkCmdPushConstants(commandBuffer, forward.blitGraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PostProcessParameters), &forward.postProcessParameters);

		vkCmdDraw(commandBuffer, 4, 1, 0, 0);

		RenderImgui();

		vkCmdEndRenderPass(commandBuffer);
	}

	void RHI::DestroyForwardRenderer() noexcept
	{
		DestroyGraphicsPipeline(forward.blitGraphicsPipeline);
		DestroyGraphicsPipeline(forward.ssaoGraphicsPipeline);
		DestroyGraphicsPipeline(forward.rtGraphicsPipeline);
		DestroyGraphicsPipeline(forward.rtCutoutGraphicsPipeline);
		DestroyGraphicsPipeline(forward.rtTransparentBackGraphicsPipeline);
		DestroyGraphicsPipeline(forward.rtTransparentFrontGraphicsPipeline);
		DestroyGraphicsPipeline(forward.envMapGraphicsPipeline);

		vkDestroyDescriptorPool(device, forward.descriptorPool, nullptr);

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			DestroyBuffer(forward.viewProjUniformBuffers[i]);
			vkDestroyFramebuffer(device, forward.rtFrameBuffers[i], nullptr);
			vkDestroyFramebuffer(device, forward.blitFrameBuffers[i], nullptr);

			vkDestroyImage(device, forward.rtColorAttachmentImages[i], nullptr);
			vkDestroyImageView(device, forward.rtColorAttachmentImageViews[i], nullptr);
			vkFreeMemory(device, forward.rtColorAttachmentImageMemories[i], nullptr);

			DestroyImage(forward.ssaoColorAttachments[i]);
			vkDestroyFramebuffer(device, forward.ssaoFrameBuffers[i], nullptr);
		}
		
		vkDestroySampler(device, forward.sampler, nullptr);
		vkDestroySampler(device, forward.cubemapSampler, nullptr);
		vkDestroySampler(device, forward.irradianceSampler, nullptr);
		vkDestroySampler(device, forward.prefilteredSampler, nullptr);
		vkDestroySampler(device, forward.SSAONoiseSampler, nullptr);

		vkDestroyImage(device, forward.rtDepthAttachmentImage, nullptr);
		vkDestroyImageView(device, forward.rtDepthAttachmentImageView, nullptr);
		vkFreeMemory(device, forward.rtDepthAttachmentMemory, nullptr);

		DestroyImage(forward.rtResolveColorAttachment);
		DestroyImage(forward.rtPositionMap);
		DestroyImage(forward.rtResolvePositionMap);
		DestroyImage(forward.rtNormalMap);
		DestroyImage(forward.rtResolveNormalMap);
		DestroyImage(forward.rtIndirectColorMap);
		DestroyImage(forward.rtResolveIndirectColorMap);
		DestroyImage(forward.SSAONoiseImage);
		DestroyBuffer(forward.SSAOKernelsUniformBuffer);

		vkDestroyRenderPass(device, forward.rtRenderPass, nullptr);
		vkDestroyRenderPass(device, forward.blitRenderPass, nullptr);
		vkDestroyRenderPass(device, forward.ssaoRenderPass, nullptr);
	}

} // namespace lux::rhi