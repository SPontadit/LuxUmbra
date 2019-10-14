#ifndef RHI_H_INCLUDED
#define RHI_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "rhi\LuxVkImpl.h"
#include "Window.h"
#include "GraphicsPipeline.h"
#include "Image.h"
#include "ForwardRenderer.h"
#include "Buffer.h"
#include "resource\Mesh.h"

namespace lux::rhi
{

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
		void RenderForward() noexcept;

		void CreateBuffer(const BufferCreateInfo& luxBufferCI, Buffer& buffer) noexcept;
		void UpdateBuffer(Buffer& buffer, void* newData) noexcept;
		void DestroyBuffer(Buffer& buffer) noexcept;

		static const uint32_t SWAPCHAIN_MIN_IMAGE_COUNT = 2;

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

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		ForwardRenderer forward;
		uint32_t frameCount;
		uint32_t currentFrame;


		void InitInstanceAndDevice(const Window& window) noexcept;
		void InitSwapchain() noexcept;
		void InitCommandBuffer() noexcept;
		void InitForwardRenderPass() noexcept;
		void InitForwardFramebuffers() noexcept;
		void InitForwardGraphicsPipelines() noexcept;
		void InitForwardDescriptorPool() noexcept;
		void InitForwardDescriptorSets() noexcept;

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const noexcept;
		VkFormat FindSupportedImageFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const noexcept;

		VkCommandBuffer BeginSingleTimeCommandBuffer() const noexcept;
		void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer) const noexcept;

		void CommandTransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) noexcept;

		void CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& luxGraphicsPipelineCI, GraphicsPipeline& graphicsPipeline) noexcept;
		void CreateShaderModule(const std::string& binaryFilePath, VkShaderModule* shaderModule) const noexcept;
		void DestroyGraphicsPipeline(GraphicsPipeline& graphicsPipeline) noexcept;

		void CreateImage(const ImageCreateInfo& luxImageCI, Image& image) noexcept;
		void DestroyImage(Image& image) noexcept;

#ifdef VULKAN_ENABLE_VALIDATION
		VkDebugReportCallbackEXT debugReportCallback;
		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
#endif // VULKAN_ENABLE_VALIDATION
	};

} // namespace lux::rhi

#endif // RHI_H_INCLUDED
