#include "rhi\RHI.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

namespace lux::rhi
{

	using namespace lux;

	ShadowMapper::ShadowMapper() noexcept
		: renderPass(VK_NULL_HANDLE),
		shadowMaps(0), framebuffers(0),
		directionalShadowMappingPipeline(),
		viewProjUniformBuffer(),
		descriptorPool(VK_NULL_HANDLE), viewProjUniformBufferDescriptorSet(VK_NULL_HANDLE)
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
		shadowMapper.shadowMaps.reserve(LIGHT_MAX_COUNT);
		shadowMapper.framebuffers.reserve(LIGHT_MAX_COUNT);

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

	void RHI::InitShadowMapperViewProjUniformBuffer() noexcept
	{
		BufferCreateInfo bufferCI = {};
		bufferCI.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferCI.size = static_cast<VkDeviceSize>(sizeof(ShadowMappingViewProjUniform));
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		CreateBuffer(bufferCI, shadowMapper.viewProjUniformBuffer);
	}

	void RHI::InitShadowMapperDescriptorPool() noexcept
	{
		VkDescriptorPoolSize shadowMappingViewProjUBODescriptorPoolSize = {};
		shadowMappingViewProjUBODescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		shadowMappingViewProjUBODescriptorPoolSize.descriptorCount = LIGHT_MAX_COUNT;

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = 1;
		descriptorPoolCI.pPoolSizes = &shadowMappingViewProjUBODescriptorPoolSize;
		descriptorPoolCI.maxSets = LIGHT_MAX_COUNT;

		CHECK_VK(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &shadowMapper.descriptorPool));
	}

	void RHI::InitShadowMapperDescriptorSets() noexcept
	{
		VkDescriptorSetAllocateInfo descriptorSetAI = {};
		descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAI.descriptorPool = shadowMapper.descriptorPool;
		descriptorSetAI.descriptorSetCount = 1;
		descriptorSetAI.pSetLayouts = &shadowMapper.directionalShadowMappingPipeline.viewDescriptorSetLayout;

		CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAI, &shadowMapper.viewProjUniformBufferDescriptorSet));

		VkDescriptorBufferInfo viewProjDescriptorBufferInfo = {};
		viewProjDescriptorBufferInfo.offset = 0;
		viewProjDescriptorBufferInfo.range = sizeof(ShadowMappingViewProjUniform);
		viewProjDescriptorBufferInfo.buffer = shadowMapper.viewProjUniformBuffer.buffer;

		VkWriteDescriptorSet writeViewProjUniformBufferDescriptorSet = {};
		writeViewProjUniformBufferDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeViewProjUniformBufferDescriptorSet.descriptorCount = 1;
		writeViewProjUniformBufferDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeViewProjUniformBufferDescriptorSet.dstBinding = 0;
		writeViewProjUniformBufferDescriptorSet.dstArrayElement = 0;
		writeViewProjUniformBufferDescriptorSet.pBufferInfo = &viewProjDescriptorBufferInfo;
		writeViewProjUniformBufferDescriptorSet.dstSet = shadowMapper.viewProjUniformBufferDescriptorSet;

		vkUpdateDescriptorSets(device, 1, &writeViewProjUniformBufferDescriptorSet, 0, nullptr);
	}

	void RHI::DestroyShadowMapper() noexcept
	{
		vkFreeDescriptorSets(device, shadowMapper.descriptorPool, 1, &shadowMapper.viewProjUniformBufferDescriptorSet);

		vkDestroyDescriptorPool(device, shadowMapper.descriptorPool, nullptr);

		DestroyBuffer(shadowMapper.viewProjUniformBuffer);

		DestroyGraphicsPipeline(shadowMapper.directionalShadowMappingPipeline);

		size_t shadowMappingResourceCount = shadowMapper.shadowMaps.size();
		for (size_t i = 0; i < shadowMappingResourceCount; i++)
		{
			vkDestroyFramebuffer(device, shadowMapper.framebuffers[i], nullptr);
			DestroyImage(shadowMapper.shadowMaps[i]);
		}

		vkDestroyRenderPass(device, shadowMapper.renderPass, nullptr);
	}

	void RHI::RenderShadowMaps(VkCommandBuffer commandBuffer, int imageIndex, scene::LightNode* shadowCastingDirectional, const std::vector<scene::MeshNode*>& meshes) noexcept
	{
		/*VkDeviceSize vertexBufferOffsets[] = { 0 };
		uint32_t meshCount = TO_UINT32_T(meshes.size());

		// Shadow mapping

		ShadowMappingViewProjUniform shadowMappingViewProjUniform = {};

		AABB lightAABB;

		glm::mat4 lightTransform = glm::toMat4(shadowCastingDirectional->GetWorldRotation());
		glm::mat4 inverseLightTransform = glm::inverse(lightTransform);

		float aabbDebugBoxScaleFactor = 1.0f / 1.28f;

		for (uint32_t i = 0; i < meshCount; i++)
		{
			scene::MeshNode* meshNode = meshes[TO_SIZE_T(i)];

			glm::mat4 localToLightTransform = inverseLightTransform * meshNode->GetWorldTransform();

			AABB meshAABB = meshNode->GetMesh().aabb;
			meshAABB.Transform(localToLightTransform);

			if (i == 1)
				lightAABB = meshAABB;
			else
				lightAABB.MakeFit(meshAABB);
		}

		float aabbx = (lightAABB.max.x - lightAABB.min.x) * 0.5f + 1.f;
		float aabby = (lightAABB.max.y - lightAABB.min.y) * 0.5f + 1.f;
		float aabbz = (lightAABB.max.z - lightAABB.min.z) * 0.5f + 1.f;

		glm::mat4 proj = glm::ortho(-aabbx, aabbx, -aabby, aabby, -aabbz, aabbz);
		proj[1][1] *= -1.f;

		glm::vec3 lightPos = (lightTransform * glm::vec4((lightAABB.min + lightAABB.max) * 0.5f, 1.0f)).xyz;
		glm::vec3 lightDir = (lightTransform * glm::vec4(0.f, 0.f, -1.f, 1.f)).xyz;

		glm::mat4 view = glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0.f, 1.f, 0.f));

		shadowMappingViewProjUniform.viewProj = proj * view;

		UpdateBuffer(shadowMapper.viewProjUniformBuffer, &shadowMappingViewProjUniform);

		forward.rtViewProjUniform.lightViewProj = shadowMappingViewProjUniform.viewProj;

		VkClearValue depthClearValue = {};
		depthClearValue.depthStencil.depth = 1.f;
		depthClearValue.depthStencil.stencil = 0;

		VkRenderPassBeginInfo shadowMappingRenderPassBI = {};
		shadowMappingRenderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		shadowMappingRenderPassBI.renderPass = shadowMapper.renderPass;
		shadowMappingRenderPassBI.framebuffer = shadowMapper.framebuffer;
		shadowMappingRenderPassBI.renderArea = { { 0, 0 }, {SHADOW_MAP_TEXTURE_SIZE, SHADOW_MAP_TEXTURE_SIZE} };
		shadowMappingRenderPassBI.clearValueCount = 1;
		shadowMappingRenderPassBI.pClearValues = &depthClearValue;

		vkCmdBeginRenderPass(commandBuffer, &shadowMappingRenderPassBI, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapper.directionalShadowMappingPipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapper.directionalShadowMappingPipeline.pipelineLayout, 0, 1, &shadowMapper.viewProjUniformBufferDescriptorSet, 0, nullptr);

		ShadowMappingModelConstant shadowMappingModelConstant = {};

		for (uint32_t i = 0; i < meshCount; i++)
		{
			scene::MeshNode* meshNode = meshes[i];

			shadowMappingModelConstant.model = meshNode->GetWorldTransform();
			vkCmdPushConstants(commandBuffer, shadowMapper.directionalShadowMappingPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, TO_UINT32_T(sizeof(ShadowMappingModelConstant)), &shadowMappingModelConstant);

			const resource::Mesh& mesh = meshNode->GetMesh();
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, vertexBufferOffsets);
			vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffer);*/
	}

	int16_t RHI::CreateLightShadowMappingResources(scene::LightType lightType) noexcept
	{
		switch (lightType)
		{
		case scene::LightType::LIGHT_TYPE_DIRECTIONAL:
		{
			size_t newResourceIndex = shadowMapper.shadowMaps.size() + 1;

			shadowMapper.shadowMaps.resize(newResourceIndex + 1);
			shadowMapper.framebuffers.resize(newResourceIndex + 1);

			ImageCreateInfo imageCI = {};
			imageCI.format = depthImageFormat;
			imageCI.width = SHADOW_MAP_TEXTURE_SIZE;
			imageCI.height = SHADOW_MAP_TEXTURE_SIZE;
			imageCI.arrayLayers = 1;
			imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageCI.subresourceRangeLayerCount = 1;
			imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;

			CreateImage(imageCI, shadowMapper.shadowMaps[newResourceIndex]);

			CommandTransitionImageLayout(shadowMapper.shadowMaps[newResourceIndex].image, depthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			VkFramebufferCreateInfo framebufferCI = {};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.renderPass = shadowMapper.renderPass;
			framebufferCI.width = SHADOW_MAP_TEXTURE_SIZE;
			framebufferCI.height = SHADOW_MAP_TEXTURE_SIZE;
			framebufferCI.layers = 1;
			framebufferCI.attachmentCount = 1;
			framebufferCI.pAttachments = &shadowMapper.shadowMaps[newResourceIndex].imageView;

			CHECK_VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &shadowMapper.framebuffers[newResourceIndex]));

			return TO_INT16_T(newResourceIndex);
		}

		default:
			return -1;
		}
	}

} // namespace lux::rhi