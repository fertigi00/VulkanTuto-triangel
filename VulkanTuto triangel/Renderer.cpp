#include <iostream>
#include "Dependencies.h"
#include "Renderer.h"
#include "Debug.h"
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <array>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;
//VerVertex input description
struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

	static vk::VertexInputBindingDescription getBindingDescription()
	{
		return { .binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex };
	}

	static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		return { {{.location = 0, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, pos)},
				 {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)}} };
	}
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

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
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
	{
		throw std::runtime_error("failed to create window surface!");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface);
}

/*std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	vk::Win32SurfaceCreateInfoKHR createInfo{.hinstance = GetModuleHandle(nullptr),
		.hwnd      = glfwGetWin32Window(window)};
	surface = instance.createWin32SurfaceKHR(createInfo);

	uint32_t queueIndex = ~0;*/

//shwapchaine 
std::vector<const char*> Renderer::GetRequiredSwapchainExtension()
{
	//uint32_t glfwSwapchainExtensionCount = 0;
	//auto glfwSwapchainExtensions = glfwGetRequiredInstanceExtensions(&glfwSwapchainExtensionCount);

	//std::vector<char const*> requiredSwapchainExtensions = { vk::KHRSwapchainExtensionName,};

	//std::vector extensionsw(glfwSwapchainExtensions, glfwSwapchainExtensions + glfwSwapchainExtensionCount);

	//extensionsw.append_range(requiredSwapchainExtensions);

	//return extensionsw;

	std::vector<char const*> requiredSwapchainExtensions = { vk::KHRSwapchainExtensionName };
	return requiredSwapchainExtensions;
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
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

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
static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}
	std::vector<char> buffer(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
	file.close();
	return buffer;
}

void Renderer::createGraphicsPipeline()
{
	vk::raii::ShaderModule shaderModule = createShaderModule(readFile("shaders/slang.spv"));
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule,  .pName = "vertMain" };
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain" };
	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo   vertexInputInfo{ .vertexBindingDescriptionCount = 1,
	.pVertexBindingDescriptions = &bindingDescription,
	.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
	.pVertexAttributeDescriptions = attributeDescriptions.data() };

	std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicState{ .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data() };

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = vk::PrimitiveTopology::eTriangleList };

	vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f };
	vk::Rect2D scissor{ vk::Offset2D{ 0, 0 }, swapChainExtent };

	vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .scissorCount = 1 };
	vk::PipelineRasterizationStateCreateInfo rasterizer{ .depthClampEnable = vk::False,
	.rasterizerDiscardEnable = vk::False,
	.polygonMode = vk::PolygonMode::eFill,
	.cullMode = vk::CullModeFlagBits::eBack,
	.frontFace = vk::FrontFace::eClockwise,
	.depthBiasEnable = vk::False,
	.lineWidth = 1.0f };
	vk::PipelineMultisampleStateCreateInfo multisampling{ .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable = vk::False,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };

	vk::PipelineColorBlendStateCreateInfo colorBlending{
	.logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ .setLayoutCount = 0, .pushConstantRangeCount = 0 };

	pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

	vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ .colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format };
	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
	{.stageCount = 2,
	 .pStages = shaderStages,
	 .pVertexInputState = &vertexInputInfo,
	 .pInputAssemblyState = &inputAssembly,
	 .pViewportState = &viewportState,
	 .pRasterizationState = &rasterizer,
	 .pMultisampleState = &multisampling,
	 .pColorBlendState = &colorBlending,
	 .pDynamicState = &dynamicState,
	 .layout = pipelineLayout,
	 .renderPass = nullptr},
	{.colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format} };

	graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

[[nodiscard]] vk::raii::ShaderModule Renderer::createShaderModule(const std::vector<char>& code) const
{
	vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
	vk::raii::ShaderModule shaderModule{ device, createInfo };
	return shaderModule;
}
/// Command Buffers
void Renderer::createCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo{ .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
								   .queueFamilyIndex = queueIndex };
	commandPool = vk::raii::CommandPool(device, poolInfo);
}

void Renderer::createCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo{ .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT };
	commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
}

void Renderer::recordCommandBuffer(uint32_t imageIndex)
{
	auto& commandBuffer = commandBuffers[frameIndex];
	transition_image_layout(
		imageIndex,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},                                                        // srcAccessMask
		vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
	);
	vk::ClearValue              clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView = swapChainImageViews[imageIndex],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor };

	vk::RenderingInfo renderingInfo = {
	.renderArea = {.offset = {0, 0}, .extent = swapChainExtent},
	.layerCount = 1,
	.colorAttachmentCount = 1,
	.pColorAttachments = &attachmentInfo };
	commandBuffer.beginRendering(renderingInfo);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
	commandBuffer.bindVertexBuffers(0, *vertexBuffer, { 0 });
	commandBuffer.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
	commandBuffers[frameIndex].bindVertexBuffers(0, *vertexBuffer, { 0 });
	commandBuffers[frameIndex].bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);
	commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	commandBuffer.endRendering();
	transition_image_layout(
		imageIndex,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
		{},                                                     // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe               // dstStage
	);
	commandBuffer.end();
}

void Renderer::transition_image_layout(
	uint32_t                imageIndex,
	vk::ImageLayout         old_layout,
	vk::ImageLayout         new_layout,
	vk::AccessFlags2        src_access_mask,
	vk::AccessFlags2        dst_access_mask,
	vk::PipelineStageFlags2 src_stage_mask,
	vk::PipelineStageFlags2 dst_stage_mask)
{
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = src_stage_mask,
		.srcAccessMask = src_access_mask,
		.dstStageMask = dst_stage_mask,
		.dstAccessMask = dst_access_mask,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = swapChainImages[imageIndex],
		.subresourceRange = {
			   .aspectMask = vk::ImageAspectFlagBits::eColor,
			   .baseMipLevel = 0,
			   .levelCount = 1,
			   .baseArrayLayer = 0,
			   .layerCount = 1} };
	vk::DependencyInfo dependency_info = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier };
	commandBuffers[frameIndex].pipelineBarrier2(dependency_info);

}
//Rendering and presentation
void Renderer::drawFrame()
{
	auto fenceResult = device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}
	device.resetFences(*inFlightFences[frameIndex]);

	auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		recreateSwapChain();
		return;
	}

	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	device.resetFences(*inFlightFences[frameIndex]);

	commandBuffers[frameIndex].reset();
	recordCommandBuffer(imageIndex);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const vk::SubmitInfo   submitInfo{ .waitSemaphoreCount = 1,
		.pWaitSemaphores = &*presentCompleteSemaphores[frameIndex],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &*commandBuffers[frameIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*renderFinishedSemaphores[imageIndex] };
	queue.submit(submitInfo, *inFlightFences[frameIndex]);

	const vk::PresentInfoKHR presentInfoKHR{ .waitSemaphoreCount = 1,
		.pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &*swapChain,
		.pImageIndices = &imageIndex };
	result = queue.presentKHR(presentInfoKHR);
	switch (result)
	{
	case vk::Result::eSuccess:
		break;
	case vk::Result::eSuboptimalKHR:
		std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
		break;
	default:
		break;        // an unexpected result is returned!
	}
	frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::createSyncObjects()
{
	assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
		inFlightFences.emplace_back(device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	}
}


///Swap chain recreation

void Renderer::cleanupSwapChain()
{
	swapChainImageViews.clear();
	swapChain = nullptr;
}

void Renderer::cleanup()
{
	cleanupSwapChain();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Renderer::recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while ((width == 0 || height == 0) && !glfwWindowShouldClose(window)) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	if (glfwWindowShouldClose(window)) {
		return;
	}

	device.waitIdle();

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
}
void Renderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}


void Renderer::createVertexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	auto [stagingBuffer, stagingBufferMemory] =
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
	memcpy(dataStaging, vertices.data(), bufferSize);
	stagingBufferMemory.unmapMemory();

	std::tie(vertexBuffer, vertexBufferMemory) =
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
}

void Renderer::createIndexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	auto [stagingBuffer, stagingBufferMemory] =
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = stagingBufferMemory.mapMemory(0, bufferSize);
	memcpy(data, indices.data(), (size_t)bufferSize);
	stagingBufferMemory.unmapMemory();

	std::tie(indexBuffer, indexBufferMemory) =
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>Renderer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
	vk::BufferCreateInfo   bufferInfo{ .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive };
	vk::raii::Buffer       buffer = vk::raii::Buffer(device, bufferInfo);
	vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
	vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties) };
	vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
	buffer.bindMemory(*bufferMemory, 0);
	return { std::move(buffer), std::move(bufferMemory) };
}

void Renderer::copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
{
	vk::CommandBufferAllocateInfo allocInfo{ .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
	vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());

	commandCopyBuffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
	commandCopyBuffer.end();

	queue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer }, nullptr);
	queue.waitIdle();
}

uint32_t Renderer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");

}
//


/// <summary>
void Renderer::createLogicalDevice()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
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
	queue = vk::raii::Queue(device, queueIndex, 0);
}
/// 
vk::Bool32 Renderer::DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cout << "Validation layer: Type [" << vk::to_string(type)<<"] - Message : " << pCallbackData->pMessage << std::endl;
	std::println(std::cout, "Validation layer : Type[{}] - Message : {}", to_string(type), pCallbackData->pMessage);

	return vk::False;
}
