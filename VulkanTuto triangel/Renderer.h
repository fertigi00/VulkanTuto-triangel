#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include"vulkan/vulkan_raii.hpp"


class Renderer
{
public:
	void CreatInstance();
	void pickPhysicalDevice();

	void SetWindow(GLFWwindow* window);
	void CreateSurface();

private:
	vk::raii::Context context;
	vk::raii::Instance instance = nullptr;
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device device = nullptr;
	vk::raii::Queue graphicsQueue = nullptr;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::SwapchainKHR swapChain = nullptr;
	
	vk::PhysicalDeviceFeatures deviceFeatures;
	std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
	GLFWwindow* window = nullptr;

	std::vector<const char*>GetRequiredExtension();
	std::vector<const char*>GetRequiredLayers();
	std::vector<const char*>GetRequiredSwapchainExtension();

	std::vector<vk::raii::ImageView> swapChainImageViews;
	std::vector<vk::Image> swapChainImages;

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
	uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);
	vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities);
	vk::SurfaceFormatKHR swapChainSurfaceFormat;
	vk::Extent2D swapChainExtent;

	bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
	void createLogicalDevice();
	void SetupDebugCallback();
	void createSwapChain(); 
	void createImageViews();
	void createGraphicsPipeline();
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT, vk::DebugUtilsMessageTypeFlagsEXT, const vk::DebugUtilsMessengerCallbackDataEXT*, void*);


};

