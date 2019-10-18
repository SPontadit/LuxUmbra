#ifndef RHI_H_INCLUDED
#define RHI_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "rhi\LuxVkImpl.h"
#include "Window.h"
#include "rhi\GraphicsPipeline.h"
#include "rhi\ForwardRenderer.h"
#include "rhi\Image.h"
#include "rhi\Buffer.h"
#include "resource\Mesh.h"
#include "resource\Material.h"
#include "scene\CameraNode.h"
#include "scene\MeshNode.h"
#include "scene\LightNode.h"

namespace lux::rhi
{
#define LIGHT_MAX_COUNT 64
#define MATERIAL_MAX_SET 64

	using namespace lux;


	struct LightBuffer
	{
		alignas(16) glm::vec4 position;
		alignas(16) glm::vec3 color;
	};

	struct LightCountPushConstant
	{
		uint32_t lightCount;
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
		void RenderForward(const scene::CameraNode* camera, const std::vector<scene::MeshNode*> meshes, const std::vector<scene::LightNode*>& lights) noexcept;

		void WaitIdle() noexcept;

		void CreateMaterial(resource::Material& material) noexcept;
		void DestroyMaterial(resource::Material& material) noexcept;

		void CreateBuffer(const BufferCreateInfo& luxBufferCI, Buffer& buffer) noexcept;
		void UpdateBuffer(Buffer& buffer, void* newData) noexcept;
		void DestroyBuffer(Buffer& buffer) noexcept;
		
		void CreateImage(const ImageCreateInfo& luxImageCI, Image& image) noexcept;
		void FillImage(const ImageCreateInfo& luxImageCI, Image& image) noexcept;
		void DestroyImage(Image& image) noexcept;

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
		VkQueue graphicsQueue;
		VkQueue presentQueue;

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

		std::vector<Buffer> lightUniformBuffers;
		LightCountPushConstant lightCountPushConstant;

		uint32_t frameCount;
		uint32_t currentFrame;

		void InitInstanceAndDevice(const Window& window) noexcept;
		void InitSwapchain() noexcept;
		void InitCommandBuffer() noexcept;
		void InitForwardRenderPass() noexcept;
		void InitForwardFramebuffers() noexcept;
		void InitForwardGraphicsPipelines() noexcept;
		void InitForwardSampler() noexcept;
		void InitForwardDescriptorPool() noexcept;
		void InitForwardDescriptorSets() noexcept;
		void InitForwardUniformBuffers() noexcept;
		void BuildLightUniformBuffers(size_t lightCount) noexcept;

		void UpdateForwardUniformBuffers(const scene::CameraNode* camera, const std::vector<resource::Material*>& materials, const std::vector<scene::LightNode*>& lights) noexcept;

		void DestroyForwardRenderer() noexcept;
		void DestroySwapchainRelatedResources() noexcept;
		void DestroyForwardGraphicsPipeline() noexcept;

		void InitImgui() noexcept;
		void RenderImgui() noexcept;

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const noexcept;
		VkFormat FindSupportedImageFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const noexcept;

		VkCommandBuffer BeginSingleTimeCommandBuffer() const noexcept;
		void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer) const noexcept;

		void CommandTransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) noexcept;

		void CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& luxGraphicsPipelineCI, GraphicsPipeline& graphicsPipeline) noexcept;
		void CreateShaderModule(const std::string& binaryFilePath, VkShaderModule* shaderModule) const noexcept;
		void DestroyGraphicsPipeline(GraphicsPipeline& graphicsPipeline) noexcept;

#ifdef VULKAN_ENABLE_VALIDATION
		VkDebugReportCallbackEXT debugReportCallback;
		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
#endif // VULKAN_ENABLE_VALIDATION
	};

} // namespace lux::rhi

#endif // RHI_H_INCLUDED
