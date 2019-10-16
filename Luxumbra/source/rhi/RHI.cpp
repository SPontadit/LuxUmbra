#include "rhi\RHI.h"

#include <array>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_vulkan.h"

#include "resource\ResourceManager.h"

namespace lux::rhi
{
	RHI::RHI() noexcept
		: isInitialized(false), instance(VK_NULL_HANDLE), surface(VK_NULL_HANDLE), physicalDevice(VK_NULL_HANDLE), device(VK_NULL_HANDLE),
		graphicsQueueIndex(UINT32_MAX), presentQueueIndex(UINT32_MAX), graphicsQueue(VK_NULL_HANDLE), presentQueue(VK_NULL_HANDLE),
		swapchainImageFormat(VK_FORMAT_UNDEFINED), swapchainExtent({ 0, 0 }), swapchainImageSubresourceRange{}, swapchain(VK_NULL_HANDLE),
		swapchainImageCount(0), swapchainImages(0), swapchainImageViews(0),
		commandPool(VK_NULL_HANDLE), commandBuffers(0),
		forward(), frameCount(0), currentFrame(0)
#ifdef VULKAN_ENABLE_VALIDATION
		, debugReportCallback(VK_NULL_HANDLE)
#endif // VULKAN_ENABLE_VALIDATION
	{

	}

	RHI::~RHI() noexcept
	{
		vkDeviceWaitIdle(device);

		vkDestroyDescriptorPool(device, imguiDescriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();

		DestroySwapchainRelatedResources();

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroySemaphore(device, presentSemaphores[i], nullptr);
			vkDestroySemaphore(device, acquireSemaphores[i], nullptr);
			vkDestroyFence(device, fences[i], nullptr);
		}

		vkDestroyDevice(device, nullptr);

#ifdef VULKAN_ENABLE_VALIDATION
		vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr);
#endif

		vkDestroySurfaceKHR(instance, surface, nullptr);
		
		vkDestroyInstance(instance, nullptr);
	}

	bool RHI::Initialize(const Window& window) noexcept
	{
		CHECK_VK(volkInitialize());

		InitInstanceAndDevice(window);

		InitSwapchain();

		InitCommandBuffer();
		
		InitForwardRenderPass();

		InitForwardFramebuffers();

		InitForwardDescriptorPool();
		
		InitForwardGraphicsPipelines();

		InitForwardUniformBuffers();

		BuildLightUniformBuffers(2);

		InitForwardDescriptorSets();

		// End

		InitImgui();

		isInitialized = true;

		return true;
	}

	void RHI::InitInstanceAndDevice(const Window& window) noexcept
	{
		// Instance

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Luxumbra_demo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Luxumbra";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> enabledExtensionNames = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	#ifdef VULKAN_ENABLE_VALIDATION
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	#endif // VULKAN_ENABLE_VALIDATION
		};

		std::vector<const char*> enabledLayerNames = {
#ifdef VULKAN_ENABLE_VALIDATION
			"VK_LAYER_LUNARG_standard_validation"
#endif // VULKAN_ENABLE_VALIDATION
		};

		VkInstanceCreateInfo instanceCI = {};
		instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCI.pApplicationInfo = &appInfo;
		instanceCI.enabledExtensionCount = TO_UINT32_T(enabledExtensionNames.size());
		instanceCI.ppEnabledExtensionNames = enabledExtensionNames.data();
		instanceCI.enabledLayerCount = TO_UINT32_T(enabledLayerNames.size());
		instanceCI.ppEnabledLayerNames = enabledLayerNames.data();

		CHECK_VK(vkCreateInstance(&instanceCI, nullptr, &instance));

		volkLoadInstance(instance);

		// Debug report callback

#ifdef VULKAN_ENABLE_VALIDATION
		VkDebugReportCallbackCreateInfoEXT debugReportCallbackCI = {};
		debugReportCallbackCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugReportCallbackCI.flags =
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
		debugReportCallbackCI.pfnCallback = VulkanDebugReportCallback;

		CHECK_VK(vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCI, nullptr, &debugReportCallback));
#endif // VULKAN_ENABLE_VALIDATION

#if defined(_WIN32)
		VkWin32SurfaceCreateInfoKHR win32SurfaceCI = {};
		win32SurfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32SurfaceCI.hinstance = GetModuleHandle(nullptr);
		win32SurfaceCI.hwnd = window.GetHandle();

		CHECK_VK(vkCreateWin32SurfaceKHR(instance, &win32SurfaceCI, nullptr, &surface));
#endif // defined(_WIN32)

		// Physical device

		uint32_t availablePhysicalDeviceCount;
		CHECK_VK(vkEnumeratePhysicalDevices(instance, &availablePhysicalDeviceCount, nullptr));

		std::vector<VkPhysicalDevice> availablePhysicalDevices(TO_SIZE_T(availablePhysicalDeviceCount));
		CHECK_VK(vkEnumeratePhysicalDevices(instance, &availablePhysicalDeviceCount, availablePhysicalDevices.data()));

		VkPhysicalDeviceProperties physicalDeviceProperties;
		std::string swapChainExtensionName(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		for (uint32_t i = 0; i < availablePhysicalDeviceCount; i++)
		{
			VkPhysicalDevice availablePhysicalDevice = availablePhysicalDevices[TO_SIZE_T(i)];

			vkGetPhysicalDeviceProperties(availablePhysicalDevice, &physicalDeviceProperties);
			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				uint32_t physicalDeviceExtensionCount;
				vkEnumerateDeviceExtensionProperties(availablePhysicalDevice, nullptr, &physicalDeviceExtensionCount, nullptr);

				std::vector<VkExtensionProperties> physicalDeviceExtensionsProperties(TO_SIZE_T(physicalDeviceExtensionCount));
				vkEnumerateDeviceExtensionProperties(availablePhysicalDevice, nullptr, &physicalDeviceExtensionCount, physicalDeviceExtensionsProperties.data());

				bool foundSwapChainExtension = false;

				for (uint32_t j = 0; j < physicalDeviceExtensionCount; j++)
				{
					const VkExtensionProperties& extensionProperties = physicalDeviceExtensionsProperties[TO_SIZE_T(j)];
					if (extensionProperties.extensionName == swapChainExtensionName)
					{
						foundSwapChainExtension = true;
						break;
					}
				}

				if (foundSwapChainExtension)
				{
					physicalDevice = availablePhysicalDevice;
					break;
				}
			}
		}

		ASSERT(physicalDevice != VK_NULL_HANDLE);

		// Device

		uint32_t queueFamilieCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilieCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamiliesProperties(TO_SIZE_T(queueFamilieCount));
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilieCount, queueFamiliesProperties.data());

		bool foundGraphicsQueue = false;
		bool foundPresentQueue = false;

		for (uint32_t i = 0; i < queueFamilieCount; i++)
		{
			VkQueueFamilyProperties queueFamilyProperties = queueFamiliesProperties[TO_SIZE_T(i)];
			if (!foundGraphicsQueue && (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) && queueFamilyProperties.queueCount > 0)
			{
				graphicsQueueIndex = i;
				foundGraphicsQueue = true;
			}

			if (!foundPresentQueue)
			{
				VkBool32 surfaceSupported;
				CHECK_VK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &surfaceSupported));

				if (surfaceSupported)
				{
					presentQueueIndex = i;
					foundPresentQueue = true;
				}
			}

			if (foundGraphicsQueue && foundPresentQueue)
				break;
		}

		float queuePriority = 1.0f;

		VkDeviceQueueCreateInfo graphicsQueueCI = {};
		graphicsQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCI.queueFamilyIndex = graphicsQueueIndex;
		graphicsQueueCI.queueCount = 1;
		graphicsQueueCI.pQueuePriorities = &queuePriority;

		std::vector<VkDeviceQueueCreateInfo> deviceQueueCIs{ graphicsQueueCI };

		if (presentQueueIndex != graphicsQueueIndex)
		{
			VkDeviceQueueCreateInfo presentQueueCI = {};
			presentQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			presentQueueCI.queueFamilyIndex = presentQueueIndex;
			presentQueueCI.queueCount = 1;
			presentQueueCI.pQueuePriorities = &queuePriority;

			deviceQueueCIs.push_back(presentQueueCI);
		}

		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};

		std::vector<const char*> deviceExtensionNames{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		std::vector<const char*> deviceLayerNames{
#ifdef VULKAN_ENABLE_VALIDATION
			"VK_LAYER_LUNARG_standard_validation"
#endif // VULKAN_ENABLE_VALIDATION
		};

		VkDeviceCreateInfo deviceCI = {};
		deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCI.queueCreateInfoCount = TO_UINT32_T(deviceQueueCIs.size());
		deviceCI.pQueueCreateInfos = deviceQueueCIs.data();
		deviceCI.pEnabledFeatures = &physicalDeviceFeatures;
		deviceCI.enabledExtensionCount = TO_UINT32_T(deviceExtensionNames.size());
		deviceCI.ppEnabledExtensionNames = deviceExtensionNames.data();
		deviceCI.enabledLayerCount = TO_UINT32_T(deviceLayerNames.size());
		deviceCI.ppEnabledLayerNames = deviceLayerNames.data();

		CHECK_VK(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));

		volkLoadDevice(device);

		vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
		vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);
	}

	void RHI::InitSwapchain() noexcept
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		CHECK_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

		swapchainExtent = surfaceCapabilities.currentExtent;

		VkImageUsageFlags swapchainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainImageUsage |= (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT) & surfaceCapabilities.supportedUsageFlags;

		uint32_t availableSurfaceFormatCount;
		CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableSurfaceFormatCount, nullptr));

		std::vector<VkSurfaceFormatKHR> availableSurfaceFormats(TO_SIZE_T(availableSurfaceFormatCount));
		CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableSurfaceFormatCount, availableSurfaceFormats.data()));

		VkSurfaceFormatKHR surfaceFormat;

		for (uint32_t i = 0; i < availableSurfaceFormatCount; i++)
		{
			const VkSurfaceFormatKHR& availableSurfaceFormat = availableSurfaceFormats[TO_SIZE_T(i)];
			if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surfaceFormat = availableSurfaceFormat;
				break;
			}
		}

		swapchainImageFormat = surfaceFormat.format;

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

		uint32_t availablePresentModeCount;
		CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &availablePresentModeCount, nullptr));

		std::vector<VkPresentModeKHR> availablePresentModes(availablePresentModeCount);
		CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &availablePresentModeCount, availablePresentModes.data()));

		for (uint32_t i = 0; i < availablePresentModeCount; i++)
		{
			VkPresentModeKHR availablePresentMode = availablePresentModes[TO_SIZE_T(i)];
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = availablePresentMode;
				break;
			}
		}

		uint32_t queueFamilyIndices[2]{ graphicsQueueIndex, presentQueueIndex };

		VkSwapchainCreateInfoKHR swapchainCI = {};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.surface = surface;
		swapchainCI.minImageCount = SWAPCHAIN_MIN_IMAGE_COUNT;
		swapchainCI.imageFormat = swapchainImageFormat;
		swapchainCI.imageColorSpace = surfaceFormat.colorSpace;
		swapchainCI.imageExtent = swapchainExtent;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageUsage = swapchainImageUsage;

		if (graphicsQueueIndex != presentQueueIndex)
		{
			swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCI.queueFamilyIndexCount = 2;
			swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCI.queueFamilyIndexCount = 0;
			swapchainCI.pQueueFamilyIndices = nullptr;
		}

		swapchainCI.preTransform = surfaceCapabilities.currentTransform;
		swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCI.presentMode = presentMode;
		swapchainCI.clipped = VK_TRUE;

		CHECK_VK(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));

		CHECK_VK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr));

		swapchainImages.resize(TO_SIZE_T(swapchainImageCount));
		CHECK_VK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data()));

		swapchainImageViews.resize(TO_SIZE_T(swapchainImageCount));

		swapchainImageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		swapchainImageSubresourceRange.baseMipLevel = 0;
		swapchainImageSubresourceRange.levelCount = 1;
		swapchainImageSubresourceRange.baseArrayLayer = 0;
		swapchainImageSubresourceRange.layerCount = 1;

		VkImageViewCreateInfo swapchainImageViewCI = {};
		swapchainImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		swapchainImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		swapchainImageViewCI.format = swapchainImageFormat;
		swapchainImageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCI.subresourceRange = swapchainImageSubresourceRange;

		for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			swapchainImageViewCI.image = swapchainImages[TO_SIZE_T(i)];

			CHECK_VK(vkCreateImageView(device, &swapchainImageViewCI, nullptr, &swapchainImageViews[TO_SIZE_T(i)]));
		}
	
		// Fences and semaphores
		VkSemaphoreCreateInfo rtSemaphoreCI = {};
		rtSemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo rtFenceCI = {};
		rtFenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		rtFenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		presentSemaphores.resize(TO_SIZE_T(swapchainImageCount));
		acquireSemaphores.resize(TO_SIZE_T(swapchainImageCount));
		fences.resize(TO_SIZE_T(swapchainImageCount));

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			CHECK_VK(vkCreateSemaphore(device, &rtSemaphoreCI, nullptr, &presentSemaphores[i]));
			CHECK_VK(vkCreateSemaphore(device, &rtSemaphoreCI, nullptr, &acquireSemaphores[i]));

			CHECK_VK(vkCreateFence(device, &rtFenceCI, nullptr, &fences[i]));
		}


		VkDescriptorPoolSize materialsUniformDescriptorPoolSize = {};
		materialsUniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		materialsUniformDescriptorPoolSize.descriptorCount = swapchainImageCount * MATERIAL_MAX_SET;

		std::array<VkDescriptorPoolSize, 1> descriptorPoolSizes = { materialsUniformDescriptorPoolSize };

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = TO_UINT32_T(descriptorPoolSizes.size());
		descriptorPoolCI.pPoolSizes = descriptorPoolSizes.data();
		descriptorPoolCI.maxSets = swapchainImageCount * MATERIAL_MAX_SET;

		CHECK_VK(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &materialDescriptorPool));
	}

	void RHI::InitCommandBuffer() noexcept
	{
		VkCommandPoolCreateInfo commandPoolCI = {};
		commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCI.queueFamilyIndex = presentQueueIndex;
		commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		CHECK_VK(vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool));


		VkCommandBufferAllocateInfo commandBufferAI = {};
		commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAI.commandPool = commandPool;
		commandBufferAI.commandBufferCount = swapchainImageCount;
		commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		commandBuffers.resize(TO_SIZE_T(swapchainImageCount));
		CHECK_VK(vkAllocateCommandBuffers(device, &commandBufferAI, commandBuffers.data()));
	}

	void RHI::InitImgui() noexcept
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiDescriptorPool);



		ImGui_ImplVulkan_InitInfo imguiInitInfo = {};
		imguiInitInfo.Instance = instance;
		imguiInitInfo.PhysicalDevice = physicalDevice;
		imguiInitInfo.Device = device;
		imguiInitInfo.QueueFamily = graphicsQueueIndex;
		imguiInitInfo.Queue = graphicsQueue;
		imguiInitInfo.PipelineCache = VK_NULL_HANDLE;
		imguiInitInfo.DescriptorPool = imguiDescriptorPool;
		imguiInitInfo.Allocator = VK_NULL_HANDLE;
		imguiInitInfo.MinImageCount = SWAPCHAIN_MIN_IMAGE_COUNT;
		imguiInitInfo.ImageCount = swapchainImageCount;

		ImGui_ImplVulkan_Init(&imguiInitInfo, forward.renderPass);

		ImGui::StyleColorsDark();

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		VkCommandBuffer buffer;
		vkAllocateCommandBuffers(device, &allocateInfo, &buffer);


		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(buffer, &beginInfo);

		ImGui_ImplVulkan_CreateFontsTexture(buffer);

		vkEndCommandBuffer(buffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &buffer);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void RHI::RenderImgui() noexcept
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[currentFrame]);
	}

	void RHI::BuildLightUniformBuffers(size_t lightCount) noexcept
	{
		BufferCreateInfo lightBufferCI = {};
		lightBufferCI.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		lightBufferCI.size = sizeof(LightBuffer) * LIGHT_MAX_COUNT;
		lightBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		lightBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		lightUniformBuffers.resize(TO_SIZE_T(swapchainImageCount));
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			CreateBuffer(lightBufferCI, lightUniformBuffers[i]);
		}
	}

	void RHI::CreateMaterial(resource::Material& material) noexcept
	{
		material.buffer.resize(TO_SIZE_T(swapchainImageCount));
		material.descriptorSet.resize(TO_SIZE_T(swapchainImageCount));


		std::vector<VkDescriptorSetLayout> materialDescriptorSetLayout(swapchainImageCount, forward.rtGraphicsPipeline.materialDescriptorSetLayout);
		VkDescriptorSetAllocateInfo materialDescriptorSetAI = {};
		materialDescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		materialDescriptorSetAI.descriptorPool = materialDescriptorPool;
		materialDescriptorSetAI.descriptorSetCount = swapchainImageCount;
		materialDescriptorSetAI.pSetLayouts = materialDescriptorSetLayout.data();

		CHECK_VK(vkAllocateDescriptorSets(device, &materialDescriptorSetAI, material.descriptorSet.data()));


		VkWriteDescriptorSet writeMaterialDescriptorSet = {};
		writeMaterialDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeMaterialDescriptorSet.descriptorCount = 1;
		writeMaterialDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeMaterialDescriptorSet.dstBinding = 0;
		writeMaterialDescriptorSet.dstArrayElement = 0;

		VkDescriptorBufferInfo materialDescriptorBufferInfo = {};
		materialDescriptorBufferInfo.offset = 0;
		materialDescriptorBufferInfo.range = sizeof(resource::MaterialParameters);


		BufferCreateInfo materialBufferCI = {};
		materialBufferCI.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		materialBufferCI.size = sizeof(resource::MaterialParameters);
		materialBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		materialBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			CreateBuffer(materialBufferCI, material.buffer[i]);

			materialDescriptorBufferInfo.buffer = material.buffer[i].buffer;
			writeMaterialDescriptorSet.pBufferInfo = &materialDescriptorBufferInfo;
			writeMaterialDescriptorSet.dstSet = material.descriptorSet[i];

			std::array<VkWriteDescriptorSet, 1> writeDescriptorSets = { writeMaterialDescriptorSet };

			vkUpdateDescriptorSets(device, TO_UINT32_T(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void RHI::DestroyMaterial(resource::Material& material) noexcept
	{
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			DestroyBuffer(material.buffer[i]);
		}
	}

	VkCommandBuffer RHI::BeginSingleTimeCommandBuffer() const noexcept
	{
		VkCommandBufferAllocateInfo commandBufferAI = {};
		commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAI.commandBufferCount = 1;
		commandBufferAI.commandPool = commandPool;

		VkCommandBuffer commandBuffer;
		CHECK_VK(vkAllocateCommandBuffers(device, &commandBufferAI, &commandBuffer));


		VkCommandBufferBeginInfo commandBufferBI = {};
		commandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CHECK_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));
	
		return commandBuffer;
	}

	void RHI::EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer) const noexcept
	{
		CHECK_VK(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		CHECK_VK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
		CHECK_VK(vkQueueWaitIdle(graphicsQueue));

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	void RHI::CommandTransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) noexcept
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;

		VkPipelineStageFlagBits srcStageMask;
		VkPipelineStageFlagBits dstStageMask;

		switch (oldLayout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				imageMemoryBarrier.srcAccessMask = 0;
				srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				break;
			
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
				break;
		}

		switch (newLayout)
		{
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
				break;
			
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				break;
			
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				
				if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
				{
					imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}

				imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				break;
		}

		vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		EndSingleTimeCommandBuffer(commandBuffer);
	}

	void RHI::WaitIdle() noexcept
	{
		vkDeviceWaitIdle(device);
	}

	void RHI::DestroySwapchainRelatedResources() noexcept
	{
		DestroyForwardRenderer();

		vkDestroyDescriptorPool(device, materialDescriptorPool, nullptr);

		vkFreeCommandBuffers(device, commandPool, 2, commandBuffers.data());

		vkDestroyCommandPool(device, commandPool, nullptr);

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
			DestroyBuffer(lightUniformBuffers[i]);
		}

		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}

	uint32_t RHI::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const noexcept
	{
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

		for (uint32_t memoryIndex = 0; memoryIndex < deviceMemoryProperties.memoryTypeCount; memoryIndex++)
		{
			if ((typeFilter & (1 << memoryIndex)) && (deviceMemoryProperties.memoryTypes[memoryIndex].propertyFlags & properties) == properties)
			{
				return memoryIndex;
			}
		}

		return UINT32_MAX;
	}

	VkFormat RHI::FindSupportedImageFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const noexcept
	{
		ASSERT(tiling == VK_IMAGE_TILING_LINEAR || tiling == VK_IMAGE_TILING_OPTIMAL);

		VkFormatProperties formatProperties;

		for (size_t i = 0, formatCount = formats.size(); i < formatCount; i++)
		{
			vkGetPhysicalDeviceFormatProperties(physicalDevice, formats[i], &formatProperties);

			switch (tiling)
			{
			case VK_IMAGE_TILING_LINEAR:
				if ((formatProperties.linearTilingFeatures & features) == features)
					return formats[i];
				break;

			case VK_IMAGE_TILING_OPTIMAL:
				if ((formatProperties.optimalTilingFeatures & features) == features)
					return formats[i];
				break;
			}
		}

		return VK_FORMAT_MAX_ENUM;
	}

#ifdef VULKAN_ENABLE_VALIDATION
	VKAPI_ATTR VkBool32 VKAPI_CALL RHI::VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
	{
		Logger::Log(layerPrefix, msg);

#ifdef _WIN32
		if (IsDebuggerPresent()) {
			OutputDebugStringA(layerPrefix);
			OutputDebugStringA(msg);
			OutputDebugStringA("\n");
		}
#endif // _WIN32
		return VK_FALSE;
	}
#endif // VULKAN_ENABLE_VALIDATION
} // namespace lux::rhi