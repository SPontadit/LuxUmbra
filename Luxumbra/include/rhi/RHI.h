#ifndef RHI_H_INCLUDED
#define RHI_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "rhi\LuxVkImpl.h"
#include "Window.h"
#include "rhi\GraphicsPipeline.h"
#include "rhi\ComputePipeline.h"
#include "rhi\ForwardRenderer.h"
#include "rhi\ShadowMapper.h"
#include "rhi\Image.h"
#include "rhi\Buffer.h"
#include "resource\Mesh.h"
#include "resource\Material.h"
#include "scene\CameraNode.h"
#include "scene\MeshNode.h"
#include "scene\LightNode.h"

namespace lux::rhi
{
#define DIRECTIONAL_LIGHT_MAX_COUNT 4
#define POINT_LIGHT_MAX_COUNT 64
#define MATERIAL_MAX_SET 128

	using namespace lux;

	struct DirectionalLightBuffer
	{
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 color;
		alignas(16) glm::mat4 viewProj;
		float shadowMapTexelSize;
		float pcfExtent;
		float pcfKernelSize;
	};

	struct PointLightBuffer
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 color;
		float radius;
	};

	struct LightCountsPushConstant
	{
		uint32_t directionalLightCount;
		uint32_t pointLightCount;
	};

	class RHI
	{
	public:
		RHI() noexcept;
		RHI(const RHI&) = delete;
		RHI(RHI&&) = delete;

		~RHI() noexcept;

		const RHI& operator=(const RHI&) = delete;
		const RHI& operator=(RHI&&) = delete;

		bool Initialize(const Window& window) noexcept;
		void Render(const scene::CameraNode* camera, const std::vector<scene::MeshNode*> meshes, const std::vector<scene::LightNode*>& lights) noexcept;

		void WaitIdle() noexcept;

		void CreateMaterial(resource::Material& material) noexcept;
		void DestroyMaterial(resource::Material& material) noexcept;

		void CreateBuffer(const BufferCreateInfo& luxBufferCI, Buffer& buffer) noexcept;
		void UpdateBuffer(Buffer& buffer, void* newData) noexcept;
		void DestroyBuffer(Buffer& buffer) noexcept;
		
		void CreateImage(const ImageCreateInfo& luxImageCI, Image& image) noexcept;
		void CreateImage(const ImageCreateInfo& luxImageCI, Image& image, VkSampler* sampler) noexcept;
		void GenerateMipChain(const ImageCreateInfo& luxImageCI, Image& image) noexcept;
		void FillImage(const ImageCreateInfo& luxImageCI, Image& image) noexcept;
		void DestroyImage(Image& image) noexcept;
		void DestroyImage(Image& image, VkSampler* sampler) noexcept;

		void CreateEnvMapDescriptorSet(Image& image) noexcept;
		
		void GenerateCubemapFromHDR(const Image& HDRSource, Image& cubemap) noexcept;
		void GenerateIrradianceFromCubemap(const Image& cubemapSource, Image& irradiance) noexcept;
		void GeneratePrefilteredFromCubemap(const Image& cubemapSource, Image& prefiltered) noexcept;
		void GenerateBRDFLut(Image& BRDFLut) noexcept;

		int16_t CreateLightShadowMappingResources(scene::LightType lightType) noexcept;

		void SetCubeMesh(std::shared_ptr<resource::Mesh> mesh) noexcept;

		void RebuildForwardGraphicsPipeline() noexcept;

		static const uint32_t SWAPCHAIN_MIN_IMAGE_COUNT = 2;
		ForwardRenderer forward;

	private:
		bool isInitialized;

		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;

		uint32_t graphicsQueueIndex;
		uint32_t presentQueueIndex;
		uint32_t computeQueueIndex;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue computeQueue;

		VkSampleCountFlagBits msaaSamples;

		VkFormat depthImageFormat;

		VkFormat swapchainImageFormat;
		VkExtent2D swapchainExtent;
		VkImageSubresourceRange swapchainImageSubresourceRange;
		VkSwapchainKHR swapchain;
		uint32_t swapchainImageCount;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		
		std::vector<VkSemaphore> presentSemaphores;
		std::vector<VkSemaphore> acquireSemaphores;

		std::vector<VkFence> fences;

		VkDescriptorPool imguiDescriptorPool;
		VkDescriptorPool materialDescriptorPool;


		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<Buffer> directionalLightUniformBuffers;
		std::vector<Buffer> pointLightUniformBuffers;
		LightCountsPushConstant lightCountsPushConstant;

		VkCommandPool computeCommandPool;
		//VkCommandBuffer computeCommandBuffer;
		//VkDescriptorPool computeDescriptorPool;
		//VkDescriptorSet generateIrradianceDescriptorSet;


		std::shared_ptr<resource::Mesh> cube;
		uint32_t frameCount;
		uint32_t currentFrame;

		ShadowMapper shadowMapper;

		void InitInstanceAndDevice(const Window& window) noexcept;
		void InitSwapchain() noexcept;
		void InitCommandBuffer() noexcept;

		void InitShadowMapperRenderPass() noexcept;
		void InitShadowMapperPipelines() noexcept;
		void InitShadowMapperDescriptorPool() noexcept;
		void InitShadowMapperDefaultResources() noexcept;

		void InitForwardRenderPass() noexcept;
		void InitForwardFramebuffers() noexcept;
		void InitForwardGraphicsPipelines(bool useCache = true) noexcept;
		void InitForwardSampler() noexcept;
		void InitForwardDescriptorPool() noexcept;
		void InitForwardDescriptorSets() noexcept;
		void InitForwardUniformBuffers() noexcept;

		void GenerateIrradianceFromCubemapFS(const Image& cubemapSource, Image& irradiance) noexcept;
		void GenerateIrradianceFromCubemapCS(const Image& cubemapSource, Image& irradiance) noexcept;
		void GeneratePrefilteredFromCubemapFS(const Image& cubemapSource, Image& prefiltered) noexcept;
		void GeneratePrefilteredFromCubemapCS(const Image& cubemapSource, Image& prefiltered) noexcept;
		void GenerateBRDFLutFS(Image& BRDFLut) noexcept;
		void GenerateBRDFLutCS(Image& BRDFLut) noexcept;

		void RenderShadowMaps(VkCommandBuffer commandBuffer, const std::vector<scene::LightNode*>& lights,const std::vector<scene::MeshNode*>& meshes) noexcept;
		void RenderForward(VkCommandBuffer commandBuffer, int imageIndex, const scene::CameraNode* camera, const std::vector<scene::MeshNode*>& meshes, const std::vector<scene::LightNode*>& lights) noexcept;
		void RenderPostProcess(VkCommandBuffer commandBuffer, int imageIndex) noexcept;

		void BuildLightUniformBuffers(size_t lightCount) noexcept;

		void GenerateCubemap(const CubeMapCreateInfo& luxCubemapCI, const Image& source, Image& image) noexcept;

		void UpdateForwardUniformBuffers(const scene::CameraNode* camera, const std::vector<resource::Material*>& materials) noexcept;

		void DestroySwapchainRelatedResources() noexcept;
		void DestroyComputeRelatedResources() noexcept;
		void DestroyShadowMapper() noexcept;
		void DestroyForwardRenderer() noexcept;
		void DestroyForwardGraphicsPipeline() noexcept;

		void InitImgui() noexcept;
		void RenderImgui() noexcept;

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const noexcept;
		VkFormat FindSupportedImageFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const noexcept;

		VkCommandBuffer BeginSingleTimeCommandBuffer() const noexcept;
		void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer) const noexcept;

		void CommandTransitionImageLayout(VkCommandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount = 1, uint32_t levelCount = 1, uint32_t baseMipLevel = 0) noexcept;
		void CommandTransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount = 1, uint32_t levelCount = 1, uint32_t baseMipLevel = 0) noexcept;

		void CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& luxGraphicsPipelineCI, GraphicsPipeline& graphicsPipeline) noexcept;
		void WriteGraphicsPipelineCacheOnDisk(const std::string& cacheFilePath, GraphicsPipeline& graphicsPipeline) noexcept;
		void CreateGraphicsPipelineCache(const std::string& pipelineCacheFilePath, GraphicsPipeline& graphicsPipeline) noexcept;
		void CreateShaderModule(const std::string& binaryFilePath, VkShaderModule* shaderModule) const noexcept;
		void DestroyGraphicsPipeline(GraphicsPipeline& graphicsPipeline) noexcept;

		void CreateComputePipeline(const ComputePipelineCreateInfo& luxComputePipelineCI, ComputePipeline& computePipeline) noexcept;
		void DestroyComputePipeline(ComputePipeline& computePipeline) noexcept;

#ifdef VULKAN_ENABLE_VALIDATION
		VkDebugReportCallbackEXT debugReportCallback;
		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
#endif // VULKAN_ENABLE_VALIDATION
	};

} // namespace lux::rhi

#endif // RHI_H_INCLUDED
