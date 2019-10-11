#ifndef RHI_H_INCLUDED
#define RHI_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "rhi\LuxVkImpl.h"
#include "Window.h"
#include "GraphicsPipeline.h"
#include "Buffer.h"

namespace lux::rhi
{

	class RHI
	{
	public:
		RHI();
		RHI(const RHI&) = delete;
		RHI(RHI&&) = delete;

		~RHI();

		const RHI& operator=(const RHI&) = delete;
		const RHI& operator=(RHI&&) = delete;

		bool Initialize(const Window& window);

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
		VkSwapchainKHR swapchain;
		uint32_t swapchainImageCount;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;

		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;

		VkRenderPass forwardRenderPass;

		enum ForwardAttachmentBindPoints : uint32_t
		{
			FORWARD_SWAPCHAIN_ATTACHMENT_BIND_POINT = 0,
			FORWARD_RT_COLOR_ATTACHMENT_BIND_POINT,
			FORWARD_RT_DEPTH_ATTACHMENT_BIND_POINT,
			FORWARD_ATTACHMENT_BIND_POINT_COUNT
		};

		enum ForwardSubpassess : uint32_t
		{
			FORWARD_SUBPASS_RENDER_TO_TARGET = 0,
			FORWARD_SUBPASS_COPY,
			FORWARD_SUBPASS_COUNT
		};

		void InitInstanceAndDevice(const Window& window);
		void InitSwapchain();
		void InitForwardRenderPass();
		void InitCommandBuffer() noexcept;

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const noexcept;
		VkFormat FindSupportedImageFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const;

		VkCommandBuffer BeginSingleTimeCommandBuffer() const noexcept;
		void EndSingleTimeCommandBuffer(VkCommandBuffer& commandBuffer) const noexcept;

		void CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& luxGraphicsPipelineCI, GraphicsPipeline& graphicsPipeline) noexcept;
		void CreateShaderModule(const std::string& binaryFilePath, VkShaderStageFlagBits pipelineStage, VkShaderModule* shaderModule) const noexcept;
		void DestroyGraphicsPipeline(GraphicsPipeline& graphicsPipeline) noexcept;

		void CreateBuffer(const BufferCreateInfo& luxBufferCI, Buffer& buffer) noexcept;
		void UpdateBuffer(Buffer& buffer, void* newData) noexcept;
		void DestroyBuffer(Buffer& buffer) noexcept;

#ifdef VULKAN_ENABLE_VALIDATION
		VkDebugReportCallbackEXT debugReportCallback;
		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
#endif // VULKAN_ENABLE_VALIDATION
	};

} // namespace lux::rhi

#endif // RHI_H_INCLUDED
