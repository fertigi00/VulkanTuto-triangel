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
	void createLogicalDevice();
	void SetupDebugCallback();
	void createSwapChain();
	void createImageViews();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createCommandPool();
	void createTextureImage();
	void createCommandBuffer();
	void drawFrame();
	void createSyncObjects();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	vk::raii::Device device = nullptr;
	bool framebufferResized = false;
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
	vk::raii::Context context;
	vk::raii::Instance instance = nullptr;
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::SwapchainKHR swapChain = nullptr;
	vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline graphicsPipeline = nullptr;
	vk::raii::CommandPool commandPool = nullptr;
	vk::raii::Semaphore presentCompleteSemaphore = nullptr;
	vk::raii::Semaphore renderFinishedSemaphore = nullptr;
	vk::raii::Fence drawFence = nullptr;
	uint32_t queueIndex = ~0;
	vk::raii::Queue queue = nullptr;

	vk::raii::Buffer vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexBufferMemory = nullptr;
	vk::raii::Buffer       indexBuffer = nullptr;
	vk::raii::DeviceMemory indexBufferMemory = nullptr;

	vk::PhysicalDeviceFeatures deviceFeatures;
	vk::raii::DescriptorPool descriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> descriptorSets;

	vk::raii::Image        textureImage = nullptr;
	vk::raii::DeviceMemory textureImageMemory = nullptr;


	GLFWwindow* window = nullptr;
	void recreateSwapChain();
	void cleanupSwapChain();
	void cleanup();

	std::vector<const char*>GetRequiredExtension();
	std::vector<const char*>GetRequiredLayers();
	std::vector<const char*>GetRequiredSwapchainExtension();
	std::vector<vk::raii::CommandBuffer> commandBuffers;
	std::vector<vk::raii::ImageView> swapChainImageViews;
	std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
	std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
	std::vector<vk::raii::Fence> inFlightFences;
	std::vector<vk::Image> swapChainImages;
	std::vector<vk::raii::Buffer> uniformBuffers;
	std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
	uint32_t frameIndex = 0;

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
	uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	

	vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities);
	vk::SurfaceFormatKHR swapChainSurfaceFormat;
	vk::Extent2D swapChainExtent;
	bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
	void recordCommandBuffer(uint32_t imageIndex);
	void transition_image_layout(uint32_t imageIndex, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask, vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask);
	void updateUniformBuffer(uint32_t currentImage);
	
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT, vk::DebugUtilsMessageTypeFlagsEXT, const vk::DebugUtilsMessengerCallbackDataEXT*, void*);

};