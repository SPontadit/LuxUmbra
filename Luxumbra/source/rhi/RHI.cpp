#include "rhi\RHI.h"

namespace lux::rhi
{
	RHI::RHI()
		: isInitialized(false), instance(VK_NULL_HANDLE), surface(VK_NULL_HANDLE), physicalDevice(VK_NULL_HANDLE), device(VK_NULL_HANDLE),
		graphicsQueueIndex(UINT32_MAX), presentQueueIndex(UINT32_MAX), graphicsQueue(VK_NULL_HANDLE), presentQueue(VK_NULL_HANDLE),
		swapchainImageFormat(VK_FORMAT_UNDEFINED), swapchainExtent({ 0, 0 }), swapchain(VK_NULL_HANDLE),
		swapchainImageCount(0), swapchainImages(0), swapchainImageViews(0),
		debugReportCallback(VK_NULL_HANDLE)
	{

	}

	RHI::~RHI()
	{

	}

	bool RHI::Initialize(const Window& window)
	{
		CHECK_VK(volkInitialize());


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
#endif
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
#endif

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
		VkDeviceQueueCreateInfo deviceQueueCI = {};
		deviceQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCI.queueFamilyIndex = graphicsQueueIndex;
		deviceQueueCI.queueCount = 1;
		deviceQueueCI.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};

		std::vector<const char*> deviceExtensionNames{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		std::vector<const char*> deviceLayerNames{
#ifdef VULKAN_ENABLE_VALIDATION
			"VK_LAYER_LUNARG_standard_validation"
#endif
		};

		VkDeviceCreateInfo deviceCI = {};
		deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCI.queueCreateInfoCount = 1;
		deviceCI.pQueueCreateInfos = &deviceQueueCI;
		deviceCI.pEnabledFeatures = &physicalDeviceFeatures;
		deviceCI.enabledExtensionCount = TO_UINT32_T(deviceExtensionNames.size());
		deviceCI.ppEnabledExtensionNames = deviceExtensionNames.data();
		deviceCI.enabledLayerCount = TO_UINT32_T(deviceLayerNames.size());
		deviceCI.ppEnabledLayerNames = deviceLayerNames.data();

		CHECK_VK(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));

		volkLoadDevice(device);

		vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
		vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);

		// Swapchain

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

		VkImageViewCreateInfo swapchainImageViewCI = {};
		swapchainImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		swapchainImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		swapchainImageViewCI.format = swapchainImageFormat;
		swapchainImageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		swapchainImageViewCI.subresourceRange.baseMipLevel = 0;
		swapchainImageViewCI.subresourceRange.levelCount = 1;
		swapchainImageViewCI.subresourceRange.baseArrayLayer = 0;
		swapchainImageViewCI.subresourceRange.layerCount = 1;

		for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			swapchainImageViewCI.image = swapchainImages[TO_SIZE_T(i)];

			CHECK_VK(vkCreateImageView(device, &swapchainImageViewCI, nullptr, &swapchainImageViews[TO_SIZE_T(i)]));
		}

		// End

		isInitialized = true;

		return true;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL RHI::VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
	{
		Logger::Log(layerPrefix, msg);

#ifdef _WIN32
		if (IsDebuggerPresent()) {
			OutputDebugStringA("[VULKAN VALIDATION] ");
			OutputDebugStringA(msg);
			OutputDebugStringA("\n");
		}
#endif
		return VK_FALSE;
	}
}