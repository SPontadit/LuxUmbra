#include "rhi\RHI.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

namespace lux::rhi
{

	using namespace lux;

	ShadowMapper::ShadowMapper() noexcept
		: renderPass(VK_NULL_HANDLE), directionalShadowMappingPipeline(), descriptorPool(VK_NULL_HANDLE),
		dummyDirectionalShadowMap(), directionalShadowMaps(0), directionalFramebuffers(0),
		directionalUniformBuffers(0), directionalUniformBufferDescriptorSets(0),
		pointShadowMaps(0), pointFramebuffers(0),
		pointUniformBuffers(0), pointUniformBufferDescriptorSets(0)
	{

	}

	void RHI::InitShadowMapperDefaultResources() noexcept
	{
		ImageCreateInfo dummyDirectionalShadowMapImageCI = {};
		dummyDirectionalShadowMapImageCI.format = depthImageFormat;
		dummyDirectionalShadowMapImageCI.width = 2;
		dummyDirectionalShadowMapImageCI.height = 2;
		dummyDirectionalShadowMapImageCI.arrayLayers = 1;
		dummyDirectionalShadowMapImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		dummyDirectionalShadowMapImageCI.subresourceRangeLayerCount = 1;
		dummyDirectionalShadowMapImageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		dummyDirectionalShadowMapImageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

		CreateImage(dummyDirectionalShadowMapImageCI, shadowMapper.dummyDirectionalShadowMap);

		CommandTransitionImageLayout(shadowMapper.dummyDirectionalShadowMap.image, depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

		shadowMapper.directionalShadowMaps.reserve(DIRECTIONAL_LIGHT_MAX_COUNT);
		shadowMapper.directionalFramebuffers.reserve(DIRECTIONAL_LIGHT_MAX_COUNT);
		shadowMapper.directionalUniformBuffers.reserve(DIRECTIONAL_LIGHT_MAX_COUNT);
		shadowMapper.directionalUniformBufferDescriptorSets.reserve(DIRECTIONAL_LIGHT_MAX_COUNT);

		shadowMapper.pointShadowMaps.reserve(POINT_LIGHT_MAX_COUNT);
		shadowMapper.pointFramebuffers.reserve(POINT_LIGHT_MAX_COUNT);
		shadowMapper.pointUniformBuffers.reserve(POINT_LIGHT_MAX_COUNT);
		shadowMapper.pointUniformBufferDescriptorSets.reserve(POINT_LIGHT_MAX_COUNT);
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
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

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
		modelPushConstantRange.size = sizeof(ShadowMappingModelConstant);
		modelPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		GraphicsPipelineCreateInfo directionalShadowMappingPipelineCI = {};
		directionalShadowMappingPipelineCI.renderPass = shadowMapper.renderPass;
		directionalShadowMappingPipelineCI.subpassIndex = 0;
		directionalShadowMappingPipelineCI.binaryVertexFilePath = "data/shaders/shadowMapping/directionalShadowMapping.vert.spv";
		directionalShadowMappingPipelineCI.vertexLayout = VertexLayout::VERTEX_POSITION_ONLY_LAYOUT;
		directionalShadowMappingPipelineCI.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		directionalShadowMappingPipelineCI.viewportWidth = TO_FLOAT(SHADOW_MAP_TEXTURE_SIZE);
		directionalShadowMappingPipelineCI.viewportHeight = TO_FLOAT(SHADOW_MAP_TEXTURE_SIZE);
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

		CreateGraphicsPipeline(directionalShadowMappingPipelineCI, shadowMapper.directionalShadowMappingPipeline);
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

	void RHI::DestroyShadowMapper() noexcept
	{
		size_t directionalLightCount = shadowMapper.directionalFramebuffers.size();
		for (size_t i = 0; i < directionalLightCount; i++)
		{
			vkDestroyFramebuffer(device, shadowMapper.directionalFramebuffers[i], nullptr);
			DestroyImage(shadowMapper.directionalShadowMaps[i]);
			DestroyBuffer(shadowMapper.directionalUniformBuffers[i]);
		}
		vkFreeDescriptorSets(device, shadowMapper.descriptorPool, TO_UINT32_T(directionalLightCount), shadowMapper.directionalUniformBufferDescriptorSets.data());

		size_t pointLightCount = shadowMapper.pointFramebuffers.size();
		for (size_t i = 0; i < pointLightCount; i++)
		{
			vkDestroyFramebuffer(device, shadowMapper.pointFramebuffers[i], nullptr);
			DestroyImage(shadowMapper.pointShadowMaps[i]);
		}
		vkFreeDescriptorSets(device, shadowMapper.descriptorPool, TO_UINT32_T(pointLightCount), shadowMapper.pointUniformBufferDescriptorSets.data());

		vkDestroyDescriptorPool(device, shadowMapper.descriptorPool, nullptr);

		DestroyGraphicsPipeline(shadowMapper.directionalShadowMappingPipeline);

		vkDestroyRenderPass(device, shadowMapper.renderPass, nullptr);

		DestroyImage(shadowMapper.dummyDirectionalShadowMap);
	}

	void RHI::RenderShadowMaps(VkCommandBuffer commandBuffer/*, int imageIndex*/, const std::vector<scene::LightNode*> lights, const std::vector<scene::MeshNode*>& meshes) noexcept
	{
		std::array<DirectionalLightBuffer, DIRECTIONAL_LIGHT_MAX_COUNT> directionalLightBuffer;
		std::array<VkDescriptorImageInfo, DIRECTIONAL_LIGHT_MAX_COUNT> directionalShadowMapsImageDescriptorInfo;

		std::array<PointLightBuffer, POINT_LIGHT_MAX_COUNT> pointLightBuffer;
		//std::array<VkDescriptorImageInfo, POINT_LIGHT_MAX_COUNT> pointShadowMapsImageDescriptorInfo;

		size_t directionalLightIndex = 0;
		size_t pointLightIndex = 0;

		size_t lightCount = lights.size();
		size_t meshCount = meshes.size();

		VkDeviceSize vertexBufferOffsets[] = { 0 };

		//float aabbDebugBoxScaleFactor = 1.f / 1.28f;

		for (size_t i = 0; i < lightCount; i++)
		{
			scene::LightNode* light = lights[i];

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

				// Update light UBO

				lightBufferEntry.direction = lightDir;
				lightBufferEntry.color = light->GetColor();
				lightBufferEntry.viewProj = proj * view;

				// RNEDER SHADOW MAP

				size_t resourceIndex = TO_SIZE_T(light->GetShadowMappingResourceIndex());

				// Update shadow mapping UBO

				DirectionalShadowMappingViewProjUniform viewProjUniform;
				viewProjUniform.viewProj = lightBufferEntry.viewProj;

				UpdateBuffer(shadowMapper.directionalUniformBuffers[resourceIndex], &viewProjUniform);

				// Update shadow maps descriptor

				VkDescriptorImageInfo& shadowMapDescriptorInfo = directionalShadowMapsImageDescriptorInfo[directionalLightIndex];
				shadowMapDescriptorInfo.imageView = shadowMapper.directionalShadowMaps[resourceIndex].imageView;
				shadowMapDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				shadowMapDescriptorInfo.sampler = forward.sampler;

				VkClearValue depthClearValue = {};
				depthClearValue.depthStencil.depth = 1.f;
				depthClearValue.depthStencil.stencil = 0;

				VkRenderPassBeginInfo shadowMappingRenderPassBI = {};
				shadowMappingRenderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				shadowMappingRenderPassBI.renderPass = shadowMapper.renderPass;
				shadowMappingRenderPassBI.framebuffer = shadowMapper.directionalFramebuffers[resourceIndex];
				shadowMappingRenderPassBI.renderArea = { { 0, 0 }, {SHADOW_MAP_TEXTURE_SIZE, SHADOW_MAP_TEXTURE_SIZE} };
				shadowMappingRenderPassBI.clearValueCount = 1;
				shadowMappingRenderPassBI.pClearValues = &depthClearValue;

				vkCmdBeginRenderPass(commandBuffer, &shadowMappingRenderPassBI, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapper.directionalShadowMappingPipeline.pipeline);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapper.directionalShadowMappingPipeline.pipelineLayout, 0, 1, &shadowMapper.directionalUniformBufferDescriptorSets[resourceIndex], 0, nullptr);

				ShadowMappingModelConstant shadowMappingModelConstant = {};

				for (size_t j = 1; j < meshCount; j += 2)
				{
					scene::MeshNode* meshNode = meshes[j];

					shadowMappingModelConstant.model = meshNode->GetWorldTransform();
					vkCmdPushConstants(commandBuffer, shadowMapper.directionalShadowMappingPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, TO_UINT32_T(sizeof(ShadowMappingModelConstant)), &shadowMappingModelConstant);

					const resource::Mesh& mesh = meshNode->GetMesh();
					vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, vertexBufferOffsets);
					vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
				}

				vkCmdEndRenderPass(commandBuffer);

				directionalLightIndex++;
			}
			break;

			case scene::LightType::LIGHT_TYPE_POINT:
			{
				PointLightBuffer& lightBufferEntry = pointLightBuffer[pointLightIndex];

				// Update light UBO

				lightBufferEntry.position = light->GetWorldPosition();
				lightBufferEntry.color = light->GetColor();

				pointLightIndex++;
			}
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

		vkUpdateDescriptorSets(device, 1, &writeDirectionalShadowMapsDescriptorSet, 0, nullptr);
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
			shadowMapper.directionalFramebuffers.resize(newResourceIndex + 1);
			shadowMapper.directionalUniformBuffers.resize(newResourceIndex + 1);
			shadowMapper.directionalUniformBufferDescriptorSets.resize(newResourceIndex + 1);

			ImageCreateInfo imageCI = {};
			imageCI.format = depthImageFormat;
			imageCI.width = SHADOW_MAP_TEXTURE_SIZE;
			imageCI.height = SHADOW_MAP_TEXTURE_SIZE;
			imageCI.arrayLayers = 1;
			imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageCI.subresourceRangeLayerCount = 1;
			imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

			CreateImage(imageCI, shadowMapper.directionalShadowMaps[newResourceIndex]);

			CommandTransitionImageLayout(shadowMapper.directionalShadowMaps[newResourceIndex].image, depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			VkFramebufferCreateInfo framebufferCI = {};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.renderPass = shadowMapper.renderPass;
			framebufferCI.width = SHADOW_MAP_TEXTURE_SIZE;
			framebufferCI.height = SHADOW_MAP_TEXTURE_SIZE;
			framebufferCI.layers = 1;
			framebufferCI.attachmentCount = 1;
			framebufferCI.pAttachments = &shadowMapper.directionalShadowMaps[newResourceIndex].imageView;

			CHECK_VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &shadowMapper.directionalFramebuffers[newResourceIndex]));

			BufferCreateInfo uniformBufferCI = {};
			uniformBufferCI.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferCI.size = sizeof(DirectionalShadowMappingViewProjUniform);
			uniformBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			uniformBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			CreateBuffer(uniformBufferCI, shadowMapper.directionalUniformBuffers[newResourceIndex]);

			VkDescriptorSetAllocateInfo descriptorSetAI = {};
			descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAI.descriptorPool = shadowMapper.descriptorPool;
			descriptorSetAI.descriptorSetCount = 1;
			descriptorSetAI.pSetLayouts = &shadowMapper.directionalShadowMappingPipeline.viewDescriptorSetLayout;
			
			CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAI, &shadowMapper.directionalUniformBufferDescriptorSets[newResourceIndex]));
			
			VkDescriptorBufferInfo viewProjDescriptorBufferInfo = {};
			viewProjDescriptorBufferInfo.offset = 0;
			viewProjDescriptorBufferInfo.range = sizeof(DirectionalShadowMappingViewProjUniform);
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

		default:
			return -1;
		}
	}

	void RHI::UpdateLightsUniformBuffers(const std::vector<scene::LightNode*>& lights) noexcept
	{
		/*size_t lightCount = std::min(lights.size(), TO_SIZE_T(LIGHT_MAX_COUNT));

		if (lightCount != lightCountPushConstant.lightCount)
		{
			lightCountPushConstant.lightCount = TO_UINT32_T(lightCount);
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

		UpdateBuffer(lightUniformBuffers[currentFrame], lightDatas.data());*/
	}

} // namespace lux::rhi