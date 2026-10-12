#include <iostream>
#include "Renderer.h"
#include "Debug.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

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
//
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
/// <summary>
/// alone bad 
/// </summary>

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
/// <summary>
/// 
/// </summary>
/// 
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
