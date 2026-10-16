#include <iostream>
#include "Dependencies.h"
#include "Renderer.h"
#include "Debug.h"
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

void Renderer::CreatInstance()
{
	vk::ApplicationInfo application_info
	{
		.pApplicationName = "Vulkan Triangle",
		.applicationVersion = VK_MAKE_VERSION(1,0,0) ,
		.pEngineName = "My engine Vulkan",
		.engineVersion = VK_MAKE_VERSION(1,0,0) ,
		.apiVersion = vk::ApiVersion14,
	};

	std::vector requiredExtensions = GetRequiredExtension();

	auto extensionProperties = context.enumerateInstanceExtensionProperties();

	for (uint32_t i = 0; i < requiredExtensions.size(); ++i)
	{
			auto compareExtensions =  [glfwExtension = requiredExtensions[i]](auto const& extensionProperty)
			{
				return strcmp(extensionProperty.extensionName, glfwExtension) == 0;
			};

			if (std::ranges::none_of(extensionProperties, compareExtensions))
			{
				throw std::runtime_error("Resuired GLFW extension not supported:  " + std::string(requiredExtensions[i]));
			}
	}

	std::vector<char const*> requiredLayers = GetRequiredLayers();

	
	auto layerProperties = context.enumerateInstanceLayerProperties();

	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, [&layerProperties](auto const& requiredLayer)
		{
			return std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty)
				{
					return strcmp(layerProperty.layerName, requiredLayer) == 0;
				});
		});

	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
	}
	//Create instance
	vk::InstanceCreateInfo create_info
	{
		.pApplicationInfo = &application_info ,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data(),
	};

	instance = vk::raii::Instance(context, create_info);
	
	SetupDebugCallback();
}
void Renderer::SetWindow(GLFWwindow* _window)
{
	window = _window;
}
//
void Renderer::pickPhysicalDevice()
{
	std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

	auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice) { return isDeviceSuitable(physicalDevice); });
	
	if (devIter == physicalDevices.end())
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	physicalDevice = *devIter;
}



// get the required extensions for the instance, including the debug utils extension if debugging is enabled, and the GLFW required extensions for window surface creation.
std::vector<const char*> Renderer::GetRequiredExtension()
{
	uint32_t glfwExtensionCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<char const*> requiredExtensions = { vk::EXTDebugUtilsExtensionName, }; //vk::KHRSwapchainExtensionName

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	extensions.append_range(requiredExtensions);

	return extensions;
}

std::vector<const char*> Renderer::GetRequiredLayers()
{
	const std::vector validationLayers = { "VK_LAYER_KHRONOS_validation" };

	std::vector<char const*> requiredLayers;

	if (Debug::EnableDebug)
		requiredLayers.append_range(validationLayers);

	return requiredLayers;
}

void Renderer::SetupDebugCallback()
{
	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

	vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

	vk::DebugUtilsMessengerCreateInfoEXT debugInfo
	{ 
		.messageSeverity = severityFlags,
		.messageType = messageTypeFlags,
		.pfnUserCallback =&DebugCallback
	};

	debugMessenger = instance.createDebugUtilsMessengerEXT(debugInfo);
}

// Query the physical device for a queue family that supports both graphics and presentation, and create a logical device with that queue family.
void Renderer::CreateSurface()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	vk::Win32SurfaceCreateInfoKHR createInfo{.hinstance = GetModuleHandle(nullptr),
		.hwnd      = glfwGetWin32Window(window)};
	surface = instance.createWin32SurfaceKHR(createInfo);

	uint32_t queueIndex = ~0;
	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
		{
			queueIndex = qfpIndex;
			break;
		}
	}
	if (queueIndex == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// query for Vulkan 1.3 features
	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{}, 
			{.shaderDrawParameters = true},
			{.dynamicRendering = true},           
			{.extendedDynamicState = true}         
	};

	// create a Device
	float                     queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
	auto requiredDeviceExtension = GetRequiredSwapchainExtension(); /// on le met dans un vecteur pour pouvoir le passer a la creation du device
	vk::DeviceCreateInfo      deviceCreateInfo{ .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
	.queueCreateInfoCount = 1,
	.pQueueCreateInfos = &deviceQueueCreateInfo,
	.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
	.ppEnabledExtensionNames = requiredDeviceExtension.data() };

	device = vk::raii::Device(physicalDevice, deviceCreateInfo);
	graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
}

//shwapchaine 
std::vector<const char*> Renderer::GetRequiredSwapchainExtension()
{
	uint32_t glfwSwapchainExtensionCount = 0;
	auto glfwSwapchainExtensions = glfwGetRequiredInstanceExtensions(&glfwSwapchainExtensionCount);

	std::vector<char const*> requiredSwapchainExtensions = { vk::KHRSwapchainExtensionName,};

	std::vector extensionsw(glfwSwapchainExtensions, glfwSwapchainExtensions + glfwSwapchainExtensionCount);

	extensionsw.append_range(requiredSwapchainExtensions);

	return extensionsw;
}

bool Renderer::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice)
{
	bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
	auto queueFamilies = physicalDevice.getQueueFamilyProperties();
	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

		auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions =
		std::ranges::all_of(GetRequiredSwapchainExtension(),
			[&availableDeviceExtensions](auto const& requiredSwapchainExtensions)
			{
				return std::ranges::any_of(availableDeviceExtensions,
					[requiredSwapchainExtensions](auto const& availableDeviceExtension)
					{ return strcmp(availableDeviceExtension.extensionName, requiredSwapchainExtensions) == 0; });
			});

	auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	bool supportsRequiredFeatures = (features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters) &&
		(features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering) &&
		(features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState);

	// Return true if the physicalDevice meets all the criteria
	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}
// a voir pour maitre dans swap_chaine plus tart SWAPCHAIN

void Renderer::createSwapChain() {
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
	swapChainExtent = chooseSwapExtent(surfaceCapabilities);
	uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
	swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
	vk::PresentModeKHR              presentMode = chooseSwapPresentMode(availablePresentModes);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{ .surface = *surface,
		.minImageCount = minImageCount,
		.imageFormat = swapChainSurfaceFormat.format,
		.imageColorSpace = swapChainSurfaceFormat.colorSpace,
		.imageExtent = swapChainExtent,
		.imageArrayLayers = 1, // nuber of layers in the image, 1 for a standard 2D image
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment, // coloer attachment  also possible that you’ll render images to a separate image first to perform operations like post-processing.
		.imageSharingMode = vk::SharingMode::eExclusive, // how to handle swap chain images that might be used across multiple queue families. There are two ways to handle images that are accessed from multiple queues
		.preTransform = surfaceCapabilities.currentTransform, //  like a 90-degree clockwise rotation or horizontal flip. To specify that you do not want any transformation, simply specify the current transformation
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque, // should be used for blending with other windows in the window system.
		.presentMode = presentMode,
		.clipped = true }; 

	swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
	swapChainImages = swapChain.getImages();
}

vk::SurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	const auto formatIt = std::ranges::find_if(
		availableFormats,
		[](const auto& format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
	return vk::SurfaceFormatKHR();
}

vk::PresentModeKHR Renderer::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
{
	assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(availablePresentModes,
		[](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
		vk::PresentModeKHR::eMailbox :
		vk::PresentModeKHR::eFifo;
}

vk::Extent2D Renderer::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
	return vk::Extent2D();
}

uint32_t Renderer::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
{
	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
	if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
	{
		minImageCount = surfaceCapabilities.maxImageCount;
	}
	return minImageCount;
}
///Image views !!!!!!!!!
void Renderer::createImageViews()
{
	assert(swapChainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo{ .viewType = vk::ImageViewType::e2D,
		.format = swapChainSurfaceFormat.format,
		.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };

	for (auto& image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		swapChainImageViews.emplace_back(device, imageViewCreateInfo);
	}
} 

//Pipeline startae PIPE
void Renderer::createGraphicsPipeline()
{

}
/// <summary>

void Renderer::createLogicalDevice()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](auto const& qfp) { return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0); });
	auto graphicsIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
	float queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = graphicsIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };

	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{},
			{.shaderDrawParameters = true},
			{.dynamicRendering = true},
			{.extendedDynamicState = true}
	};
	std::vector<const char*> requiredDeviceExtension = {
		vk::KHRSwapchainExtensionName };

	vk::DeviceCreateInfo deviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
		.ppEnabledExtensionNames = requiredDeviceExtension.data()
	};

	device = vk::raii::Device(physicalDevice, deviceCreateInfo);

	graphicsQueue = vk::raii::Queue(device, graphicsIndex, 0);
}
/// 
vk::Bool32 Renderer::DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cout << "Validation layer: Type [" << vk::to_string(type)<<"] - Message : " << pCallbackData->pMessage << std::endl;
	std::println(std::cout, "Validation layer : Type[{}] - Message : {}", to_string(type), pCallbackData->pMessage);

	return vk::False;
}
