#include "rhi\RHI.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

namespace lux::rhi
{

	using namespace lux;

	ShadowMapper::ShadowMapper() noexcept
		: renderPass(VK_NULL_HANDLE), shadowMappingPipeline(), descriptorPool(VK_NULL_HANDLE),
		directionalShadowMapIntermediate(), dummyDirectionalShadowMap(), directionalFramebuffer(VK_NULL_HANDLE), directionalShadowMaps(0),
		directionalUniformBuffers(0), directionalUniformBufferDescriptorSets(0),
		pointShadowMapIntermediate(), dummyPointShadowMap(), pointFramebuffer(VK_NULL_HANDLE), pointShadowMaps(0),
		pointUniformBuffers(0), pointUniformBufferDescriptorSets(0)
	{

	}

	void RHI::InitShadowMapperRenderPass() noexcept
	{
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = depthImageFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 0;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkRenderPassCreateInfo renderPassCI = {};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &depthAttachment;
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpass;

		CHECK_VK(vkCreateRenderPass(device, &renderPassCI, nullptr, &shadowMapper.renderPass));
	}

	void RHI::InitShadowMapperPipelines() noexcept
	{
		VkDescriptorSetLayoutBinding viewProjUniformBufferDescriptorSetLayoutBinding = {};
		viewProjUniformBufferDescriptorSetLayoutBinding.binding = 0;
		viewProjUniformBufferDescriptorSetLayoutBinding.descriptorCount = 1;
		viewProjUniformBufferDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		viewProjUniformBufferDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPushConstantRange modelPushConstantRange = {};
		modelPushConstantRange.offset = 0;
		modelPushConstantRange.size = TO_UINT32_T(sizeof(ShadowMappingModelConstant) + sizeof(uint32_t));
		modelPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		GraphicsPipelineCreateInfo directionalShadowMappingPipelineCI = {};
		directionalShadowMappingPipelineCI.renderPass = shadowMapper.renderPass;
		directionalShadowMappingPipelineCI.subpassIndex = 0;
		directionalShadowMappingPipelineCI.binaryVertexFilePath = "data/shaders/shadowMapping/shadowMapping.vert.spv";
		directionalShadowMappingPipelineCI.vertexLayout = VertexLayout::VERTEX_POSITION_ONLY_LAYOUT;
		directionalShadowMappingPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		directionalShadowMappingPipelineCI.viewportWidth = 0;
		directionalShadowMappingPipelineCI.viewportHeight = 0;
		directionalShadowMappingPipelineCI.rasterizerCullMode = VK_CULL_MODE_BACK_BIT;
		directionalShadowMappingPipelineCI.rasterizerFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		directionalShadowMappingPipelineCI.disableMSAA = VK_TRUE;
		directionalShadowMappingPipelineCI.enableDepthTest = VK_TRUE;
		directionalShadowMappingPipelineCI.enableDepthWrite = VK_TRUE;
		directionalShadowMappingPipelineCI.enableDepthBias = VK_TRUE;
		directionalShadowMappingPipelineCI.depthBiasConstantFactor = 4.f;
		directionalShadowMappingPipelineCI.depthBiasSlopeFactor = 1.5f;
		directionalShadowMappingPipelineCI.depthCompareOp = VK_COMPARE_OP_LESS;
		directionalShadowMappingPipelineCI.viewDescriptorSetLayoutBindings = { viewProjUniformBufferDescriptorSetLayoutBinding };
		directionalShadowMappingPipelineCI.pushConstants = { modelPushConstantRange };
		directionalShadowMappingPipelineCI.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		CreateGraphicsPipeline(directionalShadowMappingPipelineCI, shadowMapper.shadowMappingPipeline);
	}

	void RHI::InitShadowMapperDescriptorPool() noexcept
	{
		uint32_t lightMaxCount = DIRECTIONAL_LIGHT_MAX_COUNT + POINT_LIGHT_MAX_COUNT;

		VkDescriptorPoolSize shadowMappingUniformBuffersDescriptorPoolSize = {};
		shadowMappingUniformBuffersDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		shadowMappingUniformBuffersDescriptorPoolSize.descriptorCount = lightMaxCount;

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCI.poolSizeCount = 1;
		descriptorPoolCI.pPoolSizes = &shadowMappingUniformBuffersDescriptorPoolSize;
		descriptorPoolCI.maxSets = lightMaxCount;

		CHECK_VK(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &shadowMapper.descriptorPool));
	}

	void RHI::InitShadowMapperDefaultResources() noexcept
	{
		// Directional lights

		ImageCreateInfo directionalShadowMapIntermediateCI = {};
		directionalShadowMapIntermediateCI.format = depthImageFormat;
		directionalShadowMapIntermediateCI.width = DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
		directionalShadowMapIntermediateCI.height = DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
		directionalShadowMapIntermediateCI.arrayLayers = 1;
		directionalShadowMapIntermediateCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		directionalShadowMapIntermediateCI.subresourceRangeLayerCount = 1;
		directionalShadowMapIntermediateCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		directionalShadowMapIntermediateCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(directionalShadowMapIntermediateCI, shadowMapper.directionalShadowMapIntermediate);

		ImageCreateInfo dummyDirectionalShadowMapCI = {};
		dummyDirectionalShadowMapCI.format = depthImageFormat;
		dummyDirectionalShadowMapCI.width = 2;
		dummyDirectionalShadowMapCI.height = 2;
		dummyDirectionalShadowMapCI.arrayLayers = 1;
		dummyDirectionalShadowMapCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		dummyDirectionalShadowMapCI.subresourceRangeLayerCount = 1;
		dummyDirectionalShadowMapCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		dummyDirectionalShadowMapCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(dummyDirectionalShadowMapCI, shadowMapper.dummyDirectionalShadowMap);

		CommandTransitionImageLayout(shadowMapper.dummyDirectionalShadowMap.image, depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

		VkFramebufferCreateInfo directionalFramebufferCI = {};
		directionalFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		directionalFramebufferCI.renderPass = shadowMapper.renderPass;
		directionalFramebufferCI.width = DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
		directionalFramebufferCI.height = DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
		directionalFramebufferCI.layers = 1;
		directionalFramebufferCI.attachmentCount = 1;
		directionalFramebufferCI.pAttachments = &shadowMapper.directionalShadowMapIntermediate.imageView;

		CHECK_VK(vkCreateFramebuffer(device, &directionalFramebufferCI, nullptr, &shadowMapper.directionalFramebuffer));

		shadowMapper.directionalShadowMaps.reserve(DIRECTIONAL_LIGHT_MAX_COUNT);
		shadowMapper.directionalUniformBuffers.reserve(DIRECTIONAL_LIGHT_MAX_COUNT);
		shadowMapper.directionalUniformBufferDescriptorSets.reserve(DIRECTIONAL_LIGHT_MAX_COUNT);

		// Point lights

		ImageCreateInfo pointShadowMapIntermediateCI = {};
		pointShadowMapIntermediateCI.format = depthImageFormat;
		pointShadowMapIntermediateCI.width = POINT_SHADOW_MAP_TEXTURE_SIZE;
		pointShadowMapIntermediateCI.height = POINT_SHADOW_MAP_TEXTURE_SIZE;
		pointShadowMapIntermediateCI.arrayLayers = 1;
		pointShadowMapIntermediateCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		pointShadowMapIntermediateCI.subresourceRangeLayerCount = 1;
		pointShadowMapIntermediateCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		pointShadowMapIntermediateCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(pointShadowMapIntermediateCI, shadowMapper.pointShadowMapIntermediate);

		ImageCreateInfo dummyPointShadowMapCI = {};
		dummyPointShadowMapCI.format = depthImageFormat;
		dummyPointShadowMapCI.width = 2;
		dummyPointShadowMapCI.height = 2;
		dummyPointShadowMapCI.arrayLayers = 6;
		dummyPointShadowMapCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		dummyPointShadowMapCI.subresourceRangeLayerCount = 6;
		dummyPointShadowMapCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		dummyPointShadowMapCI.imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;

		CreateImage(dummyPointShadowMapCI, shadowMapper.dummyPointShadowMap);

		CommandTransitionImageLayout(shadowMapper.dummyPointShadowMap.image, depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 6);

		VkFramebufferCreateInfo pointFramebufferCI = {};
		pointFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		pointFramebufferCI.renderPass = shadowMapper.renderPass;
		pointFramebufferCI.width = POINT_SHADOW_MAP_TEXTURE_SIZE;
		pointFramebufferCI.height = POINT_SHADOW_MAP_TEXTURE_SIZE;
		pointFramebufferCI.layers = 1;
		pointFramebufferCI.attachmentCount = 1;
		pointFramebufferCI.pAttachments = &shadowMapper.pointShadowMapIntermediate.imageView;

		CHECK_VK(vkCreateFramebuffer(device, &pointFramebufferCI, nullptr, &shadowMapper.pointFramebuffer));

		shadowMapper.pointShadowMaps.reserve(POINT_LIGHT_MAX_COUNT);
		shadowMapper.pointUniformBuffers.reserve(POINT_LIGHT_MAX_COUNT);
		shadowMapper.pointUniformBufferDescriptorSets.reserve(POINT_LIGHT_MAX_COUNT);
	}

	void RHI::DestroyShadowMapper() noexcept
	{
		// Directional lights

		size_t directionalLightCount = shadowMapper.directionalShadowMaps.size();
		for (size_t i = 0; i < directionalLightCount; i++)
		{
			DestroyImage(shadowMapper.directionalShadowMaps[i]);
			DestroyBuffer(shadowMapper.directionalUniformBuffers[i]);
		}
		vkFreeDescriptorSets(device, shadowMapper.descriptorPool, TO_UINT32_T(directionalLightCount), shadowMapper.directionalUniformBufferDescriptorSets.data());

		vkDestroyFramebuffer(device, shadowMapper.directionalFramebuffer, nullptr);
		DestroyImage(shadowMapper.directionalShadowMapIntermediate);
		DestroyImage(shadowMapper.dummyDirectionalShadowMap);

		// Point lights

		size_t pointLightCount = shadowMapper.pointShadowMaps.size();
		for (size_t i = 0; i < pointLightCount; i++)
		{
			DestroyImage(shadowMapper.pointShadowMaps[i]);
			DestroyBuffer(shadowMapper.pointUniformBuffers[i]);
		}
		vkFreeDescriptorSets(device, shadowMapper.descriptorPool, TO_UINT32_T(pointLightCount), shadowMapper.pointUniformBufferDescriptorSets.data());

		vkDestroyFramebuffer(device, shadowMapper.pointFramebuffer, nullptr);
		DestroyImage(shadowMapper.pointShadowMapIntermediate);
		DestroyImage(shadowMapper.dummyPointShadowMap);

		// Common

		vkDestroyDescriptorPool(device, shadowMapper.descriptorPool, nullptr);

		DestroyGraphicsPipeline(shadowMapper.shadowMappingPipeline);

		vkDestroyRenderPass(device, shadowMapper.renderPass, nullptr);
	}

	void RHI::RenderShadowMaps(VkCommandBuffer commandBuffer, const std::vector<scene::LightNode*>& lights, const std::vector<scene::MeshNode*>& meshes) noexcept
	{
		std::array<DirectionalLightBuffer, DIRECTIONAL_LIGHT_MAX_COUNT> directionalLightBuffer;
		std::array<VkDescriptorImageInfo, DIRECTIONAL_LIGHT_MAX_COUNT> directionalShadowMapsImageDescriptorInfo;

		std::array<PointLightBuffer, POINT_LIGHT_MAX_COUNT> pointLightBuffer;
		std::array<VkDescriptorImageInfo, POINT_LIGHT_MAX_COUNT> pointShadowMapsImageDescriptorInfo;

		size_t directionalLightIndex = 0;
		size_t pointLightIndex = 0;

		size_t lightCount = lights.size();
		size_t meshCount = meshes.size();

		VkDeviceSize vertexBufferOffsets[] = { 0 };

		for (size_t i = 0; i < lightCount; i++)
		{
			scene::LightNode* light = lights[i];

			int16_t resourceIndex = light->GetShadowMappingResourceIndex();
			if (resourceIndex == -1)
				continue;

			switch (light->GetType())
			{

			case scene::LightType::LIGHT_TYPE_DIRECTIONAL:
			{
				if (directionalLightIndex > DIRECTIONAL_LIGHT_MAX_COUNT)
					break;

				DirectionalLightBuffer& lightBufferEntry = directionalLightBuffer[directionalLightIndex];

				// Compute AABB in light space that bounds the entire scene

				AABB lightAABB;

				glm::mat4 lightTransform = glm::toMat4(light->GetWorldRotation());
				glm::mat4 inverseLightTransform = glm::inverse(lightTransform);

				for (size_t j = 0; j < meshCount; j++)
				{
					scene::MeshNode* meshNode = meshes[j];

					glm::mat4 localtoLightTransform = inverseLightTransform * meshNode->GetWorldTransform();

					AABB meshAABB = meshNode->GetMesh().aabb;

					meshAABB.Transform(localtoLightTransform);

					if (j == 0)
						lightAABB = meshAABB;
					else
						lightAABB.MakeFit(meshAABB);
				}

				// Compute ortho proj from light to ndc space

				float aabbx = (lightAABB.max.x - lightAABB.min.x) * 0.5f;
				float aabby = (lightAABB.max.y - lightAABB.min.y) * 0.5f;
				float aabbz = (lightAABB.max.z - lightAABB.min.z) * 0.5f;

				glm::mat4 proj = glm::ortho(-aabbx, aabbx, -aabby, aabby, -aabbz, aabbz);
				proj[1][1] *= -1.f;

				// Compute view from world to light space

				glm::vec3 lightPos = (lightTransform * glm::vec4((lightAABB.min + lightAABB.max) * 0.5f, 1.0f)).xyz;
				glm::vec3 lightDir = (lightTransform * glm::vec4(0.f, 0.f, -1.f, 1.f)).xyz;

				glm::mat4 view = glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0.f, 1.f, 0.f));

				// Update light UBO & descriptor

				lightBufferEntry.direction = lightDir;
				lightBufferEntry.color = light->GetColor();
				lightBufferEntry.viewProj = proj * view;
				lightBufferEntry.shadowMapTexelSize = 1.f / DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
				lightBufferEntry.pcfExtent = 1.f;
				lightBufferEntry.pcfKernelSize = lightBufferEntry.pcfExtent * 2.0f + 1.f;
				lightBufferEntry.pcfKernelSize *= lightBufferEntry.pcfKernelSize;

				VkDescriptorImageInfo& shadowMapDescriptorInfo = directionalShadowMapsImageDescriptorInfo[directionalLightIndex];
				shadowMapDescriptorInfo.imageView = shadowMapper.directionalShadowMaps[resourceIndex].imageView;
				shadowMapDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				shadowMapDescriptorInfo.sampler = forward.sampler;

				// Update shadow mapping UBO

				ShadowMappingViewProjUniform viewProjUniform[6];
				viewProjUniform[0].viewProj = lightBufferEntry.viewProj;

				UpdateBuffer(shadowMapper.directionalUniformBuffers[resourceIndex], &viewProjUniform);

				// Render shadow map

				VkClearValue depthClearValue = {};
				depthClearValue.depthStencil.depth = 1.f;

				VkRenderPassBeginInfo shadowMappingRenderPassBI = {};
				shadowMappingRenderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				shadowMappingRenderPassBI.renderPass = shadowMapper.renderPass;
				shadowMappingRenderPassBI.framebuffer = shadowMapper.directionalFramebuffer;
				shadowMappingRenderPassBI.renderArea = { { 0, 0 }, { DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE, DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE } };
				shadowMappingRenderPassBI.clearValueCount = 1;
				shadowMappingRenderPassBI.pClearValues = &depthClearValue;

				VkViewport viewport = {};
				viewport.x = 0.f;
				viewport.y = 0.f;
				viewport.height = TO_FLOAT(DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE);
				viewport.width = TO_FLOAT(DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE);
				viewport.minDepth = 0.f;
				viewport.maxDepth = 1.f;

				VkRect2D scissor = {};
				scissor.offset = { 0, 0 };
				scissor.extent = { DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE , DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE };

				vkCmdBeginRenderPass(commandBuffer, &shadowMappingRenderPassBI, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapper.shadowMappingPipeline.pipeline);
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapper.shadowMappingPipeline.pipelineLayout, 0, 1, &shadowMapper.directionalUniformBufferDescriptorSets[resourceIndex], 0, nullptr);

				uint32_t vpIndex = 0;
				vkCmdPushConstants(commandBuffer, shadowMapper.shadowMappingPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, TO_UINT32_T(sizeof(ShadowMappingModelConstant)), sizeof(uint32_t), &vpIndex);

				ShadowMappingModelConstant shadowMappingModelConstant = {};

				for (size_t j = 0; j < meshCount; j++)
				{
					scene::MeshNode* meshNode = meshes[j];

					shadowMappingModelConstant.model = meshNode->GetWorldTransform();
					vkCmdPushConstants(commandBuffer, shadowMapper.shadowMappingPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, TO_UINT32_T(sizeof(ShadowMappingModelConstant)), &shadowMappingModelConstant);

					const resource::Mesh& mesh = meshNode->GetMesh();
					vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, vertexBufferOffsets);
					vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
				}

				vkCmdEndRenderPass(commandBuffer);

				CommandTransitionImageLayout(commandBuffer, shadowMapper.directionalShadowMaps[resourceIndex].image, depthImageFormat, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

				VkImageCopy imageCopy = {};
				imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				imageCopy.srcSubresource.layerCount = 1;
				imageCopy.srcSubresource.baseArrayLayer = 0;
				imageCopy.srcSubresource.mipLevel = 0;
				imageCopy.srcOffset = { 0, 0, 0 };

				imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				imageCopy.dstSubresource.layerCount = 1;
				imageCopy.dstSubresource.baseArrayLayer = 0;
				imageCopy.dstSubresource.mipLevel = 0;
				imageCopy.dstOffset = { 0, 0, 0 };

				imageCopy.extent.width = DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
				imageCopy.extent.height = DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
				imageCopy.extent.depth = 1;

				vkCmdCopyImage(commandBuffer, shadowMapper.directionalShadowMapIntermediate.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, shadowMapper.directionalShadowMaps[resourceIndex].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

				CommandTransitionImageLayout(commandBuffer, shadowMapper.directionalShadowMaps[resourceIndex].image, depthImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

				directionalLightIndex++;
			}
			break;

			case scene::LightType::LIGHT_TYPE_POINT:
			{
				if (pointLightIndex > POINT_LIGHT_MAX_COUNT)
					break;

				PointLightBuffer& lightBufferEntry = pointLightBuffer[pointLightIndex];

				// Update light UBO & descriptor

				lightBufferEntry.position = light->GetWorldPosition();
				lightBufferEntry.color = light->GetColor();
				lightBufferEntry.radius = light->GetRadius();

				VkDescriptorImageInfo& shadowMapDescriptorInfo = pointShadowMapsImageDescriptorInfo[pointLightIndex];
				shadowMapDescriptorInfo.imageView = shadowMapper.pointShadowMaps[resourceIndex].imageView;
				shadowMapDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				shadowMapDescriptorInfo.sampler = forward.sampler;

				// Update shadowMappingUBO

				glm::vec3 lightPos = light->GetWorldPosition();
				glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.f, 0.1f, light->GetRadius());
				proj[1][1] *= -1.f;

				ShadowMappingViewProjUniform viewProjUniformBuffer[6];

				glm::mat4 view = glm::lookAt(lightPos, lightPos + glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
				viewProjUniformBuffer[0].viewProj = proj * view;

				view = glm::lookAt(lightPos, lightPos + glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
				viewProjUniformBuffer[1].viewProj = proj * view;

				view = glm::lookAt(lightPos, lightPos + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
				viewProjUniformBuffer[2].viewProj = proj * view;

				view = glm::lookAt(lightPos, lightPos + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
				viewProjUniformBuffer[3].viewProj = proj * view;

				view = glm::lookAt(lightPos, lightPos + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));
				viewProjUniformBuffer[4].viewProj = proj * view;

				view = glm::lookAt(lightPos, lightPos + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
				viewProjUniformBuffer[5].viewProj = proj * view;

				UpdateBuffer(shadowMapper.pointUniformBuffers[resourceIndex], viewProjUniformBuffer);

				// Render shadow map

				VkClearValue depthClearValue = {};
				depthClearValue.depthStencil.depth = 1.f;

				VkRenderPassBeginInfo shadowMappingRenderPassBI = {};
				shadowMappingRenderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				shadowMappingRenderPassBI.renderPass = shadowMapper.renderPass;
				shadowMappingRenderPassBI.framebuffer = shadowMapper.pointFramebuffer;
				shadowMappingRenderPassBI.renderArea = { { 0, 0 }, { POINT_SHADOW_MAP_TEXTURE_SIZE, POINT_SHADOW_MAP_TEXTURE_SIZE } };
				shadowMappingRenderPassBI.clearValueCount = 1;
				shadowMappingRenderPassBI.pClearValues = &depthClearValue;

				VkImageCopy imageCopy = {};
				imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				imageCopy.srcSubresource.layerCount = 1;
				imageCopy.srcSubresource.baseArrayLayer = 0;
				imageCopy.srcSubresource.mipLevel = 0;
				imageCopy.srcOffset = { 0, 0, 0 };

				imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				imageCopy.dstSubresource.layerCount = 1;
				imageCopy.dstSubresource.mipLevel = 0;
				imageCopy.dstOffset = { 0, 0, 0 };

				imageCopy.extent.width = POINT_SHADOW_MAP_TEXTURE_SIZE;
				imageCopy.extent.height = POINT_SHADOW_MAP_TEXTURE_SIZE;
				imageCopy.extent.depth = 1;

				VkViewport viewport = {};
				viewport.x = 0.f;
				viewport.y = 0.f;
				viewport.height = TO_FLOAT(POINT_SHADOW_MAP_TEXTURE_SIZE);
				viewport.width = TO_FLOAT(POINT_SHADOW_MAP_TEXTURE_SIZE);
				viewport.minDepth = 0.f;
				viewport.maxDepth = 1.f;

				VkRect2D scissor = {};
				scissor.offset = { 0, 0 };
				scissor.extent = { POINT_SHADOW_MAP_TEXTURE_SIZE , POINT_SHADOW_MAP_TEXTURE_SIZE };

				CommandTransitionImageLayout(commandBuffer, shadowMapper.pointShadowMaps[resourceIndex].image, depthImageFormat, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);

				for (uint32_t i = 0; i < 6; i++)
				{
					vkCmdBeginRenderPass(commandBuffer, &shadowMappingRenderPassBI, VK_SUBPASS_CONTENTS_INLINE);

					vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapper.shadowMappingPipeline.pipeline);
					vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
					vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapper.shadowMappingPipeline.pipelineLayout, 0, 1, &shadowMapper.pointUniformBufferDescriptorSets[resourceIndex], 0, nullptr);

					vkCmdPushConstants(commandBuffer, shadowMapper.shadowMappingPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, TO_UINT32_T(sizeof(ShadowMappingModelConstant)), sizeof(uint32_t), &i);

					ShadowMappingModelConstant shadowMappingModelConstant = {};

					for (size_t j = 0; j < meshCount; j++)
					{
						scene::MeshNode* meshNode = meshes[j];

						shadowMappingModelConstant.model = meshNode->GetWorldTransform();
						vkCmdPushConstants(commandBuffer, shadowMapper.shadowMappingPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, TO_UINT32_T(sizeof(ShadowMappingModelConstant)), &shadowMappingModelConstant);

						const resource::Mesh& mesh = meshNode->GetMesh();
						vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, vertexBufferOffsets);
						vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
						vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
					}

					vkCmdEndRenderPass(commandBuffer);

					imageCopy.dstSubresource.baseArrayLayer = i;
					vkCmdCopyImage(commandBuffer, shadowMapper.pointShadowMapIntermediate.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, shadowMapper.pointShadowMaps[resourceIndex].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
				}

				CommandTransitionImageLayout(commandBuffer, shadowMapper.pointShadowMaps[resourceIndex].image, depthImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 6);

				pointLightIndex++;
			}
			break;

			default:
				ASSERT(false);
				break;
			}
		}

		lightCountsPushConstant.directionalLightCount = TO_UINT32_T(directionalLightIndex);
		lightCountsPushConstant.pointLightCount = TO_UINT32_T(pointLightIndex);

		for (; directionalLightIndex < DIRECTIONAL_LIGHT_MAX_COUNT; directionalLightIndex++)
		{
			VkDescriptorImageInfo& shadowMapDescriptorInfo = directionalShadowMapsImageDescriptorInfo[directionalLightIndex];
			shadowMapDescriptorInfo.imageView = shadowMapper.dummyDirectionalShadowMap.imageView;
			shadowMapDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			shadowMapDescriptorInfo.sampler = forward.sampler;
		}

		for (; pointLightIndex < POINT_LIGHT_MAX_COUNT; pointLightIndex++)
		{
			VkDescriptorImageInfo& shadowMapDescriptorInfo = pointShadowMapsImageDescriptorInfo[pointLightIndex];
			shadowMapDescriptorInfo.imageView = shadowMapper.dummyPointShadowMap.imageView;
			shadowMapDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			shadowMapDescriptorInfo.sampler = forward.sampler;
		}

		UpdateBuffer(directionalLightUniformBuffers[currentFrame], directionalLightBuffer.data());
		UpdateBuffer(pointLightUniformBuffers[currentFrame], pointLightBuffer.data());

		VkWriteDescriptorSet writeDirectionalShadowMapsDescriptorSet = {};
		writeDirectionalShadowMapsDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDirectionalShadowMapsDescriptorSet.descriptorCount = DIRECTIONAL_LIGHT_MAX_COUNT;
		writeDirectionalShadowMapsDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDirectionalShadowMapsDescriptorSet.dstBinding = 3;
		writeDirectionalShadowMapsDescriptorSet.dstArrayElement = 0;
		writeDirectionalShadowMapsDescriptorSet.pImageInfo = directionalShadowMapsImageDescriptorInfo.data();
		writeDirectionalShadowMapsDescriptorSet.dstSet = forward.rtViewDescriptorSets[currentFrame];

		VkWriteDescriptorSet writePointShadowMapsDescriptorSet = {};
		writePointShadowMapsDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writePointShadowMapsDescriptorSet.descriptorCount = POINT_LIGHT_MAX_COUNT;
		writePointShadowMapsDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writePointShadowMapsDescriptorSet.dstBinding = 4;
		writePointShadowMapsDescriptorSet.dstArrayElement = 0;
		writePointShadowMapsDescriptorSet.pImageInfo = pointShadowMapsImageDescriptorInfo.data();
		writePointShadowMapsDescriptorSet.dstSet = forward.rtViewDescriptorSets[currentFrame];

		std::array<VkWriteDescriptorSet, 2> descriptorSetWrites = {
			writeDirectionalShadowMapsDescriptorSet,
			writePointShadowMapsDescriptorSet
		};

		vkUpdateDescriptorSets(device, TO_UINT32_T(descriptorSetWrites.size()), descriptorSetWrites.data(), 0, nullptr);
	}

	int16_t RHI::CreateLightShadowMappingResources(scene::LightType lightType) noexcept
	{
		switch (lightType)
		{
		case scene::LightType::LIGHT_TYPE_DIRECTIONAL:
		{
			size_t newResourceIndex = shadowMapper.directionalShadowMaps.size();

			if (newResourceIndex > DIRECTIONAL_LIGHT_MAX_COUNT)
				return -1;

			shadowMapper.directionalShadowMaps.resize(newResourceIndex + 1);
			shadowMapper.directionalUniformBuffers.resize(newResourceIndex + 1);
			shadowMapper.directionalUniformBufferDescriptorSets.resize(newResourceIndex + 1);

			Image& shadowMap = shadowMapper.directionalShadowMaps[newResourceIndex];

			ImageCreateInfo imageCI = {};
			imageCI.format = depthImageFormat;
			imageCI.width = DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
			imageCI.height = DIRECTIONAL_SHADOW_MAP_TEXTURE_SIZE;
			imageCI.arrayLayers = 1;
			imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageCI.subresourceRangeLayerCount = 1;
			imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

			CreateImage(imageCI, shadowMap);

			CommandTransitionImageLayout(shadowMap.image, depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

			BufferCreateInfo uniformBufferCI = {};
			uniformBufferCI.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferCI.size = sizeof(ShadowMappingViewProjUniform);
			uniformBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			uniformBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			CreateBuffer(uniformBufferCI, shadowMapper.directionalUniformBuffers[newResourceIndex]);

			VkDescriptorSetAllocateInfo descriptorSetAI = {};
			descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAI.descriptorPool = shadowMapper.descriptorPool;
			descriptorSetAI.descriptorSetCount = 1;
			descriptorSetAI.pSetLayouts = &shadowMapper.shadowMappingPipeline.viewDescriptorSetLayout;
			
			CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAI, &shadowMapper.directionalUniformBufferDescriptorSets[newResourceIndex]));
			
			VkDescriptorBufferInfo viewProjDescriptorBufferInfo = {};
			viewProjDescriptorBufferInfo.offset = 0;
			viewProjDescriptorBufferInfo.range = sizeof(ShadowMappingViewProjUniform);
			viewProjDescriptorBufferInfo.buffer = shadowMapper.directionalUniformBuffers[newResourceIndex].buffer;
			
			VkWriteDescriptorSet writeViewProjUniformBufferDescriptorSet = {};
			writeViewProjUniformBufferDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeViewProjUniformBufferDescriptorSet.descriptorCount = 1;
			writeViewProjUniformBufferDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeViewProjUniformBufferDescriptorSet.dstBinding = 0;
			writeViewProjUniformBufferDescriptorSet.dstArrayElement = 0;
			writeViewProjUniformBufferDescriptorSet.pBufferInfo = &viewProjDescriptorBufferInfo;
			writeViewProjUniformBufferDescriptorSet.dstSet = shadowMapper.directionalUniformBufferDescriptorSets[newResourceIndex];
			
			vkUpdateDescriptorSets(device, 1, &writeViewProjUniformBufferDescriptorSet, 0, nullptr);

			return TO_INT16_T(newResourceIndex);
		}

		case scene::LightType::LIGHT_TYPE_POINT:
		{
			size_t newResourceIndex = shadowMapper.pointShadowMaps.size();

			if (newResourceIndex > POINT_LIGHT_MAX_COUNT)
				return -1;

			shadowMapper.pointShadowMaps.resize(newResourceIndex + 1);
			shadowMapper.pointUniformBuffers.resize(newResourceIndex + 1);
			shadowMapper.pointUniformBufferDescriptorSets.resize(newResourceIndex + 1);

			Image& shadowMap = shadowMapper.pointShadowMaps[newResourceIndex];

			ImageCreateInfo imageCI = {};
			imageCI.format = depthImageFormat;
			imageCI.width = POINT_SHADOW_MAP_TEXTURE_SIZE;
			imageCI.height = POINT_SHADOW_MAP_TEXTURE_SIZE;
			imageCI.arrayLayers = 6;
			imageCI.mipmapCount = 1;
			imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageCI.subresourceRangeLayerCount = 6;
			imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;

			CreateImage(imageCI, shadowMap);

			CommandTransitionImageLayout(shadowMap.image, depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 6);

			BufferCreateInfo uniformBufferCI = {};
			uniformBufferCI.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferCI.size = 6 * sizeof(ShadowMappingViewProjUniform);
			uniformBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			uniformBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			CreateBuffer(uniformBufferCI, shadowMapper.pointUniformBuffers[newResourceIndex]);

			VkDescriptorSetAllocateInfo descriptorSetAI = {};
			descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAI.descriptorPool = shadowMapper.descriptorPool;
			descriptorSetAI.descriptorSetCount = 1;
			descriptorSetAI.pSetLayouts = &shadowMapper.shadowMappingPipeline.viewDescriptorSetLayout;

			CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAI, &shadowMapper.pointUniformBufferDescriptorSets[newResourceIndex]));

			VkDescriptorBufferInfo viewProjDescriptorBufferInfo = {};
			viewProjDescriptorBufferInfo.offset = 0;
			viewProjDescriptorBufferInfo.range = 6 * sizeof(ShadowMappingViewProjUniform);
			viewProjDescriptorBufferInfo.buffer = shadowMapper.pointUniformBuffers[newResourceIndex].buffer;

			VkWriteDescriptorSet writeViewProjUniformBufferDescriptorSet = {};
			writeViewProjUniformBufferDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeViewProjUniformBufferDescriptorSet.descriptorCount = 1;
			writeViewProjUniformBufferDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeViewProjUniformBufferDescriptorSet.dstBinding = 0;
			writeViewProjUniformBufferDescriptorSet.dstArrayElement = 0;
			writeViewProjUniformBufferDescriptorSet.pBufferInfo = &viewProjDescriptorBufferInfo;
			writeViewProjUniformBufferDescriptorSet.dstSet = shadowMapper.pointUniformBufferDescriptorSets[newResourceIndex];

			vkUpdateDescriptorSets(device, 1, &writeViewProjUniformBufferDescriptorSet, 0, nullptr);

			return TO_INT16_T(newResourceIndex);
		}

		default:
			ASSERT(false);
			return -1;
		}
	}

} // namespace lux::rhi